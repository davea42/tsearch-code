/* Copyright (c) 2014, David Anderson
All rights reserved.

Redistribution and use in source and binary forms, with
or without modification, are permitted provided that the
following conditions are met:

    Redistributions of source code must retain the above
    copyright notice, this list of conditions and the following
    disclaimer.

    Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials
    provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

/* Testing tsearch etc.

    tsearch [--std] inputfile ...

    If --std is not given then only extra
    tests are done.

*/

#include "config.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif /* HAVE_STDINT_H */
#include <errno.h>
#include "dwarf_tsearch.h"


/*  These defines rename the call targets to Unix standard
    names (or to nothing where there is no standard version).
*/
#ifdef LIBC_TSEARCH
#define _GNU_SOURCE /* for tdestroy */
#define __USE_GNU   /* tdestroy */
#include <search.h>
#define dwarf_tsearch(a,b,c) tsearch(a,b,c)
#define dwarf_tfind(a,b,c) tfind(a,b,c)
#define dwarf_tdelete(a,b,c) tdelete(a,b,c)
#define dwarf_twalk(a,b) twalk(a,b)
#define dwarf_tdestroy(a,b) tdestroy(a,b)
#define dwarf_tdump(a,c,d)
#define dwarf_initialize_search_hash(a,b,c)
#define DW_VISIT VISIT
#define dwarf_preorder  preorder
#define dwarf_postorder postorder
#define dwarf_endorder  endorder
#define dwarf_leaf      leaf
#endif /* LIBC_TSEARCH */

#define REPORTING
/* The struct is trivially usable to implement a set or
   map (mapping an integer to a string).
   The following struct is the example basis
   because that is the capability I wanted to use
   for -std.
   mt_compare_func() and the  free function mt_free_func()
   (passed in to tsearch calls) know what data is involved.
*/

struct example_tentry {
    unsigned mt_key;
    /* When using this as a set of mt_key the mt_name
    field is set to 0 (NULL). */
    char * mt_name;
};
/*  Used to hold test data.
    It would be a much better testing regime
    to add two flags here. One indicating pass/fail
    for normal interfaces (as with use of
    struct example_tentry here), one indicating pass/fail
    for -byvalue runs (-byvalue has limitations
    in that its impossible to detect if an add
    is a duplicate or a new add).
    That is good reason to avoid -byvalue in most
    situations. */
struct myacts {
    char action_;
    unsigned addr_;
};


/*  Another example of tree content is a simple value.
    Since the tree contains a pointer for each object
    we save, we can only directly save a value that fits
    in a pointer.
*/
typedef unsigned VALTYPE;

enum insertorder {
    increasing,
    decreasing,
    balanced
};

static int
applybypointer(struct myacts *m,
    const char *msg,
    int hideactions,
    int printwalk,
    int dumpeverystage);
int(*applyby)(struct myacts *m,
    const char *msg,
    int hideactions,
    int printwalk,
    int dumpeverystage) = applybypointer;


static int g_hideactions = 0;
static const int increaseorder[] = {1,2,3,4,5,6};
static const int decreaseorder[] = {6,5,4,3,2,1};
/* The following a pseudo-random order. */
static const int balanceorder[] =  {3,6,2,5,4,1};

/* From real code exposing a bug: */
static struct myacts sequence1[] = {
{'a', 0x33c8},
{'a', 0x34d8},
{'a', 0x35c8},
{'a', 0x3640},
{'a', 0x3820},
{'a', 0x38d0},
{'a', 0x3958},
{'a', 0x39e8},
{'a', 0x3a78},
{'a', 0x3b08},
{'a', 0x3b98},
{'a', 0x3c28},
{'a', 0x3cb8},
{'d', 0x3c28},
{'a', 0x3d48},
{'d', 0x3cb8},
{'a', 0x3dd8},
{'d', 0x3d48},
{'a', 0x3e68},
{'d', 0x3dd8},
{'a', 0x3ef8},
{'a', 0x3f88},
{'d', 0x3e68},
{'a', 0x4018},
{'d', 0x3ef8},
{0,0}
};

static struct myacts sequence2[] = {
{'a', 0x931d2e0},
{'a', 0x931d340},
{'a', 0x931d378},
{'a', 0x931d3b8},
{'a', 0x931d0b0},
{'a', 0x931f7f8},
{'d', 0x931f7f8},
{'d', 0x931d378},
{'a', 0x931d378},
{'a', 0x931f7f8},
{'d', 0x931f7f8},
{'a', 0x931f7f8},
{'d', 0x931f7f8},
{'a', 0x931f9a8},
{'a', 0x931f9f0},
{'a', 0x931fa38},
{'a', 0x93224c0},
{'a', 0x93224f0},
{'a', 0x9322538},
{'a', 0x9322568},
{'a', 0x93225b0},
{'a', 0x93225e0},
{'a', 0x9322628},
{'a', 0x9322658},
{'a', 0x93226a0},
{'a', 0x93226d0},
{'d', 0x93224c0},
{'d', 0x9322538},
{'d', 0x93225b0},
{'d', 0x9322628},
{'d', 0x93226a0},
{'a', 0x931f918},
{'d', 0x931f918},
{'a', 0x931f918},
{0,0}
};

/*  This test is meant to ensure we can add a value with
    key of zero. */
static struct myacts sequence3[] = {
{'a', 0x0},
{'a', 0xffffffff},
{'a', 0xffff},
{0,0}
};

static struct myacts sequential64[] = {
{'a', 1},
{'a', 2},
{'a', 3},
{'a', 4},
{'a', 5},
{'a', 6},
{'a', 7},
{'a', 8},
{'a', 9},
{'a', 10},
{'a', 11},
{'a', 12},
{'a', 13},
{'a', 14},
{'a', 15},
{'a', 16},
{'a', 17},
{'a', 18},
{'a', 19},
{'a', 20},
{'a', 21},
{'a', 22},
{'a', 23},
{'a', 24},
{'a', 25},
{'a', 26},
{'a', 27},
{'a', 28},
{'a', 29},
{'a', 30},
{'a', 31},
{'a', 32},
{'a', 33},
{'a', 34},
{'a', 35},
{'a', 36},
{'a', 37},
{'a', 38},
{'a', 39},
{'a', 40},
{'a', 41},
{'a', 42},
{'a', 43},
{'a', 44},
{'a', 45},
{'a', 46},
{'a', 47},
{'a', 48},
{'a', 49},
{'a', 50},
{'a', 51},
{'a', 52},
{'a', 53},
{'a', 54},
{'a', 55},
{'a', 56},
{'a', 57},
{'a', 58},
{'a', 59},
{'a', 60},
{'a', 61},
{'a', 62},
{'a', 63},
{'a', 64},
{0,0}
};

static int runstandardtests = 0;
static char *wordspath = 0;
/* ============begin correctness test ===========*/

static int
get_record_id(enum insertorder ord,int indx)
{
    int i = 0;
    switch(ord) {
        case increasing:
            i = increaseorder[indx];
            break;
        case decreasing:
            i = decreaseorder[indx];
            break;
        case balanced:
            i = balanceorder[indx];
            break;
        default:
            printf("FAIL, internal error in test code\n");
            exit(1);
    }
    return i;
}

/* We allow a NULL name so this struct acts sort of like a set
   and sort of like a map.
*/
static struct example_tentry *
make_example_tentry(unsigned k,char *name)
{
    struct example_tentry *mt =
        (struct example_tentry *)calloc(
            sizeof(struct example_tentry),1);
    if (!mt) {
        printf("calloc fail\n");
        exit(1);
    }
    mt->mt_key = k;
    if (name) {
        mt->mt_name = strdup(name);
    }
    return mt;
}
static void
mt_free_func(void *mt_data)
{
    struct example_tentry *m = mt_data;
    if (!m) {
        return;
    }
    free(m->mt_name);
    free(mt_data);
    return;
}
#ifdef HASHSEARCH
static DW_TSHASHTYPE
mt_hashfunc(const void *keyp)
{
    /* our key here is particularly simple. */
    const struct example_tentry *ml = keyp;
    return ml->mt_key;
}

#endif /* HASHSEARCH */
static void printlevel(int level)
{
    int len = 0;
    int targetlen = 4 + level;
    int shownlen = 0;
    char number[10];
    len = snprintf(number,sizeof(number),"<%d>",level);
    printf("%s",number);
    shownlen = len;
    while(shownlen < targetlen) {
        putchar(' ');
        ++shownlen;
    }
}

static int
mt_compare_func(const void *l, const void *r)
{
    const struct example_tentry *ml = l;
    const struct example_tentry *mr = r;
    if (ml->mt_key < mr->mt_key) {
        return -1;
    }
    if (ml->mt_key > mr->mt_key) {
        return 1;
    }
    return 0;
}
static void
walk_entry(const void *mt_data,DW_VISIT x,int level)
{
    const struct example_tentry *m =
        *(const struct example_tentry **)mt_data;
    printlevel(level);
    printf("Walk on node %s %u %s  \n",
        x == dwarf_preorder?"preorder":
        x == dwarf_postorder?"postorder":
        x == dwarf_endorder?"endorder":
        x == dwarf_leaf?"leaf":
        "unknown",
        m->mt_key,m->mt_name);
    return;
}

#ifndef LIBC_TSEARCH
static char *
mt_keyprint(const void *v)
{
    static char buf[50];
    const struct example_tentry *mt =
        (const struct example_tentry *)v;
    buf[0] = 0;
    snprintf(buf,sizeof(buf),"0x%08x",(unsigned)mt->mt_key);
    return buf;
}
#endif /* LIBC_TSEARCH */

static int
insertrecsbypointer(int max, void **tree,
    const enum insertorder order)
{
    int indx = 0;
    for (indx = 0 ; indx < max ; ++indx) {
        int i = 0;
        int k = 0;
        char kbuf[40];
        char dbuf[60];
        struct example_tentry *mt = 0;
        struct example_tentry *retval = 0;

        i = get_record_id(order,indx);
        snprintf(kbuf,sizeof(kbuf),"%u",i);
        strcpy(dbuf," data for ");
        strcat(dbuf,kbuf);
        printf("insertrec %d\n",i);
        /*  Do it twice so we have test the case where
            tsearch adds and one
            where it finds an existing record. */

        for (k = 0; k < 2 ;++k) {
            mt = make_example_tentry(i,dbuf);
            errno = 0;
            /* tsearch adds an entry if its not present already. */
            retval = dwarf_tsearch(mt,tree, mt_compare_func  );
            if (retval == 0) {
                printf("FAIL ENOMEM in search on  %d, "
                    "give up insertrecsbypointer\n",i);
                exit(1);
            } else {
                struct example_tentry *re = 0;
                re = *(struct example_tentry **)retval;
                if (re != mt) {
                    if (!k) {
                        printf("FAIL found existing an error %u\n",i);
                        mt_free_func(mt);
                        return 1;
                    } else {
                        printf("found existing ok  %u\n",i);
                    }
                    /*  Prevents data leak: mt was
                        already present. */
                    mt_free_func(mt);
                } else {
                    if (!k) {
                        printf("insert new ok %u\n",i);
                    } else {
                        printf("FAIL new found but expected "
                            "existing %u\n",i);
                    }
                    /* New entry mt was added. */
                }
            }
        }
    }
    return 0;
}

static int
findrecsbypointer(int max,int findexpected,
    const void **tree,
    const enum insertorder order)
{
    int indx = 0;
    for (indx = 0 ; indx < max ; ++indx) {
        char kbuf[40];
        char dbuf[60];
        struct example_tentry *mt = 0;
        struct example_tentry *retval = 0;
        int i = 0;
        dbuf[0] = 0;
        kbuf[0] = 0;

        i = get_record_id(order,indx);
        snprintf(kbuf,sizeof(kbuf),"%u",i);
        mt = make_example_tentry(i,dbuf);
        printf("findrec %d\n",i);
        retval = dwarf_tfind(mt,(void *const*)tree,mt_compare_func);
        if (!retval) {
            if (indx < findexpected) {
                mt_free_func(mt);
                printf("FAIL tfind on %s is FAILURE\n",kbuf);
                return 1;
            } else {
                printf("Not found with tfind on %s is ok\n",kbuf);
            }
        } else {
            printf("found ok %u\n",i);
            if (indx >= findexpected) {
                mt_free_func(mt);
                printf("FAIL: found with tfind on %s is FAILURE\n",
                    kbuf);
                return 1;
            } else {
                printf("Found with tfind on %s is ok\n",kbuf);
            }
        }
        mt_free_func(mt);
    }
    return 0;
}
/* The dodump flag is so we can distinguish binarysearch actions
   from the binarysearch with eppinger mods easily in the test output.
   The difference is slight and not significant,
   but the difference is what we look for when we look.
*/
static int
delrecsbypointer(int max,int findexpected, void **tree,
    const enum insertorder order,int dodump)
{
    int indx = 0;
    for (indx = 0; indx < max; indx++) {
        struct example_tentry *mt = 0;
        struct example_tentry *re3 = 0;
        void *r = 0;

        int i = 0;

        i = get_record_id(order,indx);
        mt = make_example_tentry(i,0);
        printf("delrec %d\n",i);
        r = dwarf_tfind(mt,(void *const*)tree,mt_compare_func);
        if (r) {
            /*  This is what tdelete will delete.
                tdelete just removes the reference from
                the tree, it does not actually delete
                the memory for the entry itself.
                In fact there is no way to know for sure
                what was done just given the return
                from tdelete.  You just just assume the delete
                worked and use the tfind result to delete
                your contents if you want to.*/
            re3 = *(struct example_tentry **)r;
            if (indx < findexpected) {
                ;
            } else {
                mt_free_func(mt);
                printf("FAIL delrecsbypointer should not have found "
                    "record to delete for %d\n",i);
                return 1;
            }
            r = dwarf_tdelete(mt,tree,mt_compare_func);
            if (! *tree) {
                printf("tree itself now empty\n");
            }
            /* We don't want the 'test' node left around. */
            if (r) {
                /*  If the node deleted was root, r
                    is really the new root, not the parent.
                    Or r is non-null but bogus.
                    (so don't print).
                    */
                printf("tdelete returned parent or something.\n");
            } else {
                printf("tdelete returned NULL, tree now empty.\n");
#ifdef HASHSEARCH
                printf("Only really means some hash chain is "
                    "now empty.\n");
#endif /* HASHSEARCH */
            }
            mt_free_func(mt);
            mt_free_func(re3);
        } else {
            if (indx >= findexpected) {
                ;
            } else {
                mt_free_func(mt);
                printf("FAIL delrecsbypointer should have found "
                    "record to delete for %d\n",i);
                return 1;
            }
            /* There is no node like this to delete. */
            /* We don't want the 'test' node left around. */
            mt_free_func(mt);
        }
        if (dodump) {
            dwarf_tdump( *tree,mt_keyprint,"In Delrecs");
        }
        dwarf_twalk( *tree,walk_entry);
    }
    return 0;
}

/*  mt must point to data in static storage for this to
    make any sense.
    Malloc()ed data or unique static data for this instance mt
    points at.
    For example, if there was an immobile array and
    make_example_tentry() somehow
    selected a unique entry.
*/
static int
insertonebypointer(void **tree, unsigned long addr,int ct)
{
    struct example_tentry *mt = 0;
    void *retval = 0;
    mt = make_example_tentry(addr,0);
    /* tsearch adds an entry if its not present already. */
    retval = dwarf_tsearch(mt,tree, mt_compare_func  );
    if (retval == 0) {
        printf("FAIL ENOMEM in search on rec %d adr  0x%lu,"
            " error in insertonebypointer\n",
            ct,(unsigned long)addr);
        exit(1);
    } else {
        struct example_tentry *re = 0;
        re = *(struct example_tentry **)retval;
        if (re != mt) {
            /* Found existing, error. */
            printf("insertonebypointer rec %d addr %lu 0x%lx "
                "found record preexisting, error\n",
                ct,
                (unsigned long)addr,
                (unsigned long)addr);
            mt_free_func(mt);
            return 1;
        } else {
            /* inserted new entry, make sure present. */
#ifndef FULL_SPEED_RUN
            struct example_tentry *mt2 = make_example_tentry(addr,0);
            retval = dwarf_tfind(mt2,tree,mt_compare_func);
            mt_free_func(mt2);
            if (!retval) {
                printf("insertonebypointer record %d addr 0x%lu "
                    "failed to add as desired,"
                    " error\n",
                    ct,(unsigned long)addr);
                return 1;
            }
#endif /* FULL_SPEED_RUN */
        }
    }
    return 0;
}

/*  For tfind and tdelete one can use static data and take its address
    for mt instead of using malloc/free.
*/
static int
deleteonebypointer(void **tree, unsigned addr,int ct)
{
    struct example_tentry *mt = 0;
    struct example_tentry *re3 = 0;
    void *r = 0;
    int err=0;

    mt = make_example_tentry(addr,0);
    r = dwarf_tfind(mt,(void *const*)tree,mt_compare_func);
    if (r) {
        re3 = *(struct example_tentry **)r;
        dwarf_tdelete(mt,tree,mt_compare_func);
        mt_free_func(mt);
        mt_free_func(re3);
    } else {
        printf("deleteonebypointer could not find rec %d ! "
            "error! addr"
            " 0x%lx\n",
            ct,(unsigned long)addr);
        mt_free_func(mt);
        err = 1;
    }
    return err;
}

#ifdef HASHSEARCH
/* Only needed for hash based search in a tsearch style. */
#define INITTREE(x,y) x = dwarf_initialize_search_hash(&(x),(y),0)
#else
#define INITTREE(x,y)
#endif /* HASHSEARCH */

static const char *
describe_action(char a)
{
    static const char* ad = "add    ";
    static const char* de = "delete ";
    static const char* un = "unknown";
    switch(a) {
    case 'a': return ad;
    case 'd': return de;
    }
    return un;
}

static int
applybypointer(struct myacts *m,
    const char *msg,
    int hideactions,
    UNUSEDARG int printwalk,
    UNUSEDARG int dumpeverystage)
{

    unsigned ct = 1;
    void *treesq1 = 0;
    int errcount = 0;

    INITTREE(treesq1,mt_hashfunc);
    printf("special sequence applybypointer %s\n",msg);
    for (; m->action_ != 0; m++,ct++) {
        if (!hideactions) {
            printf("Action %2u: %s 0x%x val 0x%x\n",ct,
                describe_action(m->action_),
                m->action_,m->addr_);
        }
        if (m->action_ == 'a') {
            errcount += insertonebypointer(&treesq1,m->addr_,ct);
            continue;
        }
        if (m->action_ == 'd') {
            errcount += deleteonebypointer(&treesq1,m->addr_,ct);
            continue;
        }
        printf("Fail applybypointer, bad action %s entry %d.\n",
            msg,ct);
        return 1;
    }
    dwarf_tdestroy(treesq1,mt_free_func);
    return errcount;

}


static int
standard_tests(void)
{
    void *tree1 = 0;
    int errcount = 0;

    if (applyby == applybypointer) {
        printf("Test with increasing input\n");
        INITTREE(tree1,mt_hashfunc);

        errcount += insertrecsbypointer(3,&tree1,increasing);
        errcount += findrecsbypointer(6,3,(const void **)&tree1,
            increasing);
        dwarf_twalk(tree1,walk_entry);
        dwarf_tdump(tree1,mt_keyprint,
            "Dump Tree from increasing input");
        errcount += delrecsbypointer(6,3,&tree1,increasing,0);
#ifdef HASHSEARCH
        dwarf_tdestroy(tree1,mt_free_func);
        tree1 = 0;
#endif
        if (tree1) {
            printf("FAIL: delrecsbypointer of increasing did "
                "not empty the tree.\n");
            exit(1);
        }

        printf("Test twalk with empty tree\n");
        dwarf_twalk(tree1,walk_entry);

        INITTREE(tree1,mt_hashfunc);
        printf("Insert decreasing, try tdestroy\n");
        errcount += insertrecsbypointer(6,&tree1,decreasing);
        dwarf_twalk(tree1,walk_entry);
        dwarf_tdestroy(tree1,mt_free_func);
        tree1 = 0;

        INITTREE(tree1,mt_hashfunc);
        printf("Now test with decreasing input and test twalk "
            "and tdelete\n");
        errcount += insertrecsbypointer(5,&tree1,decreasing);
        errcount += findrecsbypointer(6,5,(const void **)&tree1,
            decreasing);
        dwarf_twalk(tree1,walk_entry);
        dwarf_tdump(tree1,mt_keyprint,
            "Dump Tree from decreasing input");
        errcount += delrecsbypointer(6,5,&tree1,decreasing,0);
#ifdef HASHSEARCH
        dwarf_tdestroy(tree1,mt_free_func);
        tree1 = 0;
#endif
        if (tree1) {
            printf("FAIL: delrecsbypointer of decreasing "
                "did not empty the tree.\n");
            exit(1);
        }

        INITTREE(tree1,mt_hashfunc);
        printf("Now test with balanced input and test twalk and "
            "tdelete\n");
        errcount += insertrecsbypointer(4,&tree1,balanced);
        errcount += findrecsbypointer(6,4,(const void **)&tree1,
            balanced);
        dwarf_twalk(tree1,walk_entry);
        dwarf_tdump(tree1,mt_keyprint,
            "Dump Tree from balanced input");
        errcount += delrecsbypointer(6,4,&tree1,balanced,1);
#ifdef HASHSEARCH
        dwarf_tdestroy(tree1,mt_free_func);
        tree1 = 0;
#endif
        if (tree1) {
            printf("FAIL: delrecsbypointer of balanced did not "
                "empty the tree.\n");
            exit(1);
        }

        dwarf_twalk(tree1,walk_entry);
        if (errcount > 0) {
            printf("FAIL tsearch test.\n");
            exit(1);
        }

        errcount += applyby(&sequence1[0],"Sequence 1",
            g_hideactions,0,0);
        errcount += applyby(&sequence2[0],"Sequence 2, a",
            g_hideactions,0,0);
    } else 
    {
        errcount += applyby(&sequence2[0],"Sequence 2, b",
            g_hideactions,0,0);
        errcount += applyby(&sequence3[0],"Sequence 3",
            g_hideactions,0,0);
        errcount += applyby(&sequential64[0],"Sequential 64",
            g_hideactions,1,0);
    }
    return errcount;
}
/* ============end correctness test ===========*/
/* ============begin large test for timing===========*/
#ifdef REPORTING
struct reportdata_s {
   unsigned r_wordcount;
   unsigned r_uniquecount;
   unsigned r_repeatcount;
} reportcount;
struct reportdata_s zerorc;
static void 
print_counts(void)
{
    printf("Counts: words: %u uniqwords %u repeatcount %u\n",
        reportcount.r_wordcount,
        reportcount.r_uniquecount,
        reportcount.r_repeatcount);
}
#else /* !REPORTING */
static void 
print_counts(void)
{
    return;
}
#endif /* REPORTING */
unsigned current_entry_number = 0;
struct str_entry {
    char *   str_key;
    unsigned str_entrynumber;
    char *   str_name;
};
static struct str_entry *
make_str_entry(char *key ,char *name)
{
    struct str_entry *mt =
        (struct str_entry *)calloc(sizeof(struct str_entry),1);
    if (!mt) {
        printf("calloc fail\n");
        exit(1);
    }
    mt->str_key = strdup(key);
    mt->str_entrynumber = current_entry_number;
    if (name) {
        mt->str_name = strdup(name);
    }
    return mt;
}
static void
str_free_func(void *str_data)
{
    struct str_entry *m = str_data;
    if (!m) {
        return;
    }
    free(m->str_name);
    free(m->str_key);
    free(str_data);
    return;
}
#ifdef HASHSEARCH
/* "Bernstein hash function" or the "DJB hash function" */
static uint32_t /* must be a 32 - bit integer type */
hash(unsigned char * str)
{
    uint32_t hash = 5381;
    int c  = 0;
    while ((c = *str++)) {
        hash = hash * 33 + c ;
    }
    return hash;
}
static DW_TSHASHTYPE
str_hashfunc(const void *keyp)
{
    const struct str_entry *l = 
        (struct str_entry *)keyp;
    return hash(l->str_key);
}

#endif /* HASHSEARCH */
static int
str_compare_func(const void *l, const void *r)
{
    const struct str_entry *ml = (struct str_entry *)l;
    const struct str_entry *mr = (struct str_entry *)r;
    return  strcmp(ml->str_key,mr->str_key);
}
#if 0
static void
str_walk_entry(const void *str_data,DW_VISIT x,int level)
{
    const struct str_entry *m =
        *(const struct str_entry **)str_data;
    printlevel(level);
    printf("Walk on node %s %s entrynum %u %s  \n",
        x == dwarf_preorder?"preorder":
        x == dwarf_postorder?"postorder":
        x == dwarf_endorder?"endorder":
        x == dwarf_leaf?"leaf":
        "unknown",
        m->str_key,m->str_entrynumber,m->str_name);
    return;
}
#endif /* 0 */
#if 0
#ifndef LIBC_TSEARCH
static char buf[1000];
static char *
str_keyprint(const void *v)
{
    const struct str_entry *str =
        (const struct str_entry *)v;
    buf[0] = 0;
    snprintf(buf,sizeof(buf),"%s",str->str_key);
    return buf;
}
#endif /* LIBC_TSEARCH */
#endif /* 0 */

void *wordtree = 0;

static void
init_word_tree(void)
{
     INITTREE(wordtree,str_hashfunc);
}
static void
destroy_word_tree(void)
{
     dwarf_tdestroy(wordtree,str_free_func);
     wordtree = 0;
}
static void
insert_word_in_tree(char *name)
{
    struct str_entry *newword = 0;
    void * r = 0;
    struct str_entry *key_deref = 0;

    newword = make_str_entry(name,name);
#if 0
    r = dwarf_tfind(newword,&wordtree,str_compare_func);
    if (r) {
       str_free_func(r); /* wrong? */
       return;
    }
#endif
    r = dwarf_tsearch(newword,&wordtree,str_compare_func);
    if (!r) {
        printf("tsearch failed. Something wrong\n");
        exit(1);
    }
    key_deref = *(struct str_entry **)r;
#ifdef REPORTING
    reportcount.r_wordcount++;
#endif /* REPORTING */
    if (key_deref == newword) {
        /* We added in this key */
        ++current_entry_number;
#ifdef REPORTING
        reportcount.r_uniquecount++;
#endif /* REPORTING */
    } else {
        /*  We found an existing */
        str_free_func(newword);
#ifdef REPORTING
        reportcount.r_repeatcount++;
#endif /* REPORTING */
    }
}

static size_t
find_size_of_chars(char *fsc_buf)
{
     char *cp = fsc_buf;
     size_t len = 0;
     int c = *cp;
     while (c && c != ' ' && c != '\n') {
         ++len;
         ++cp;
         c = *cp;
     }
     return len;
}

static char readbuf[1000];
static int
run_timing_test(char *path)
{
    FILE *f = 0;
    size_t readlen = 0;

    f = fopen(path,"r");
    if (!f) {
        printf("Cannot open %s, Giving up.\n",path);
        exit(1);
    }
    for (;;) {
        char *res = 0;

        res = fgets(readbuf,sizeof(readbuf),f);
        if (!res) {
            if (feof(f)) {
                break;
            }
            /* ERROR */
            printf("Read error, giving up\n");
            exit(1);
        } 
        readlen = find_size_of_chars(readbuf);
        readbuf[readlen] = 0;
        if (readbuf[0]) {
            insert_word_in_tree(readbuf);
        }
 
    }
    print_counts();
    fclose(f);
    return 0;
}


/* ============end large test for timing===========*/
static void
readargs(int argc, char **argv)
{
    int i = 1;
    if (argc < 2) {
        /* No arguments, take defaults. */
        return;
    }
    if (!strcmp(argv[i],"--std")) {
        runstandardtests = 1;
        ++i;
        
    }
    if (i >= argc) {
        return;
    }
    wordspath = argv[i];
    return;
}

int
main(int argc, char **argv)
{
    int errcount = 0;
    applyby = applybypointer;
    readargs(argc,argv);

    if (runstandardtests) {
        errcount += standard_tests();
    }
    if (errcount) {
        printf("FAIL std tests");
    }
    /* Do extra tests here */
#ifdef REPORTING
    reportcount = zerorc;
#endif /* REPORTING */
    if (wordspath) {
        init_word_tree();
        run_timing_test(wordspath);
        destroy_word_tree();
    }
    if (errcount) {
        exit(1);
    }
    printf("PASS tsearch test.\n");
    exit(0);
}
