/* Copyright (c) 2013-2021, David Anderson
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

/*  The interfaces follow tsearch (See the Single
    Unix Specification) but the implementation is
    written without reference to the source of any
    version of tsearch.

    See http://www.prevanders.net/tsearch.html
    for information and an example of use.

    Based on Knuth, chapter 6.2.2, algorithm T and
    Algorithm D.
*/

#include "config.h"
#include "stdlib.h" /* for free() */
#include <stdio.h> /* for printf */
#ifdef HAVE_STDINT_H
#include <stdint.h> /* for uintptr_t */
#endif /* HAVE_STDINT_H */
#define Dwarf_Unsigned unsigned long long
#ifdef _WIN32
#define DW_PR_DUx "I64x"
#else
#define DW_PR_DUx "llx"
#endif /* DW_PR defines */
#include "dwarf_tsearch.h"

/*  INVARIANT: The head node has no user data.

    head->llink is null, head->rlink points to the real user
    top node (root of the user tree).
    So the physical top node we call 'head'. No user data.
    The user top node we call 'root' here. It has a user key.

    Though we intend that head->rlink be non-NULL
    except briefly when a tdelete removes the last
    node (in which case we remove the head too before
    returning) the code is a bit cautious and tests
    for a non-NULL head->rlink.

*/

struct ts_entry {
    /*  Keyptr points to a pointer to a record the user saved, the
        user record contains the user's key itself
        and perhaps more.  */
    const void *keyptr;
    struct ts_entry * llink;
    struct ts_entry * rlink;
};

/* Not needed for this set of functions. */
void *
dwarf_initialize_search_hash( void **treeptr,
    UNUSEDARG DW_TSHASHTYPE(*hashfunc)(const void *key),
    UNUSEDARG unsigned long size_estimate)
{
    return *treeptr;
}

/*  For debugging.  Prints the level number and indents 1 space
    per level.   That won't work very well for a deep tree, so perhaps
    we should clamp at some number of indent spaces? */
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

/*  For debugging. This prints the nodes with the parent
    (in each case) in between the children.  So it is a
    tree with root at the left. */
static void
dumptree_inner(const struct ts_entry *t,
    char *(* keyprint)(const void *),
    const char *descr, int level)
{
    const char *v = "";
    if (!t) {
        return;
    }
    dumptree_inner(t->rlink,keyprint,"left ",level+1);
    if (t->keyptr) {
        v = keyprint(t->keyptr);
    }
    printlevel(level);
    printf("0x%08" DW_PR_DUx
        " <keyptr 0x%08" DW_PR_DUx
        "> <%s %s> <l 0x%08" DW_PR_DUx
        "> <r 0x%08" DW_PR_DUx "> %s\n",
        (Dwarf_Unsigned)(uintptr_t)t,
        (Dwarf_Unsigned)(uintptr_t)t->keyptr,
        t->keyptr?"key ":"null",
        v,
        (Dwarf_Unsigned)(uintptr_t)t->llink,
        (Dwarf_Unsigned)(uintptr_t)t->rlink,
        descr);
    dumptree_inner(t->llink,keyprint,"right",level+1);
}
static void
setlink(struct ts_entry*t,int a,struct ts_entry *x)
{
    if (a < 0) {
        t->llink = x;
    } else {
        t->rlink = x;
    }
}

static struct ts_entry*
getlink(struct ts_entry*t,int a)
{
    if (a < 0) {
        return(t->llink);
    }
    return(t->rlink);
}

/*  Dumping the tree to stdout. */
void
dwarf_tdump(const void*rootin,
    char *(* keyprint)(const void *),
    const char *msg)
{
    const struct ts_entry *head = (const struct ts_entry *)rootin;
    const struct ts_entry *root = 0;
    if (!head) {
        printf("dwarf_tdump null tree ptr : %s\n",msg);
        return;
    }
    root = head->rlink;
    if (!root) {
        printf("dwarf_tdump empty tree : %s\n",msg);
        return;
    }
    printf("dwarf_tdump tree head : 0x%08" DW_PR_DUx
        " %s\n",
        (Dwarf_Unsigned)(uintptr_t)head,msg);
    printf("dwarf_tdump tree root : 0x%08" DW_PR_DUx
        " %s\n",
        (Dwarf_Unsigned)(uintptr_t)root,msg);
    dumptree_inner(root,keyprint,"top",0);
}

static struct ts_entry *
allocate_ts_entry(const void *key)
{
    struct ts_entry *e = (struct ts_entry *)
        malloc(sizeof(struct ts_entry));
    if (!e) {
        return NULL;
    }
    e->keyptr = key;
    e->llink = 0;
    e->rlink = 0;
    return e;
}

/* Knuth step T5, the insert. */
static struct ts_entry *
tsearch_insert_k(const void *key,int kc,
    struct ts_entry *p)
{
    struct ts_entry *e = allocate_ts_entry(key);
    if (!e) {
        /* out of memory */
        return NULL;
    }
    setlink(p,kc,e);
    /* Non-NULL means inserted. */
    return e;
}

/* Knuth step T5. */
static struct ts_entry *
tsearch_inner_do_insert(const void *key,
    int kc,
    int * inserted,
    struct ts_entry* p)
{
    struct ts_entry *q = 0;
    q =  tsearch_insert_k(key,kc,p);
    if (q) {
        *inserted = 1;
    }
    return q;
}

/*  Algorithm T of Knuth 6.2.2.2
    key is pointer to a user data area containing the key
    and possibly more.

    We iterate like Knuth does, but using for (;;) instead
    of go-to.  */
static struct ts_entry *
tsearch_inner( const void *key, struct ts_entry* localrootp,
    int (*compar)(const void *, const void *),
    int*inserted)
{
    struct ts_entry* p = localrootp;
    for (;;) {
        struct ts_entry *r = 0;
        /* T2. */
        int kc = compar(key,p->keyptr);
        if (kc < 0) {
            /* T3. */
            struct ts_entry *l = p->llink;
            if (l) {
                p = l;
                continue;
            }
            /* T5 */
            r = tsearch_inner_do_insert(key,kc,inserted,p);
            return r;
        } else if (kc > 0 ) {
            /* T4. */
            struct ts_entry *r2 = p->rlink;
            if (r2) {
                p = r2;
                continue;
            }
            /* T5 */
            r =  tsearch_inner_do_insert(key,kc,inserted,p);
            return r;
        }
        /* K = KEY(P) in Knuth. */
        /* kc == 0, we found the entry we search for. */
        return p;
    }
    /* Can never get here. */
    return 0;
}

/* Search and, if missing, insert. */
void *
dwarf_tsearch(const void *key, void **headpin,
    int (*compar)(const void *, const void *))
{
    struct ts_entry *head = 0;
    struct ts_entry *root = 0;
    struct ts_entry *r = 0;
    int inserted = 0;

    if (!headpin) {
        return NULL;
    }
    head = (struct ts_entry *)*headpin;
    if (head) {
        root = head->rlink;
    }
    if (!head || !root) {
        int allocatedhead = 0;
        if (!head) {
            head = allocate_ts_entry(0);
            allocatedhead = 1;
        }
        if (!head) {
            return NULL;
        }
        root = allocate_ts_entry(key);
        if (!root) {
            if (allocatedhead) {
                free(head);
            }
            return NULL;
        }
        head->rlink = root;
        *headpin = head;
        return (void *)(&(root->keyptr));
    }
    root = head->rlink;
    r = tsearch_inner(key,root,compar,&inserted);
    if (!r) {
        return NULL;
    }
    /*  Was this found or inserted?
        Value is the same either way, but the pointer to return
        is not the same! */
    /* Discards const.  Required by the interface definition. */
    return (void *)&(r->keyptr);
}

/* Search. */
void *
dwarf_tfind(const void *key, void *const*headppin,
    int (*compar)(const void *, const void *))
{
    struct ts_entry *head = 0;
    struct ts_entry **proot = 0;
    struct ts_entry *root  = 0;
    struct ts_entry *p     = 0;

    if (!headppin) {
        return NULL;
    }
    head = (struct ts_entry *)*headppin;
    if (!head) {
        return NULL;
    }
    proot = &head->rlink;
    root = *proot;
    if (!root) {
        return NULL;
    }
    p = root;
    while(p) {
        int kc = compar(key,p->keyptr);
        if (!kc) {
            return (void *)&(p->keyptr);
        }
        p  = getlink(p,kc);
    }
    return NULL;
}

void *
dwarf_tdelete(const void *key, void **headin,
    int (*compar)(const void *, const void *))
{
    struct ts_entry *phead  = 0;
    struct ts_entry **rootp = 0;
    struct ts_entry *root   = 0;
    struct ts_entry *p      = 0;

    /*  If a leaf is found, we have to null a parent link
        or the root */
    struct ts_entry * parentp = 0;
    int parentcomparv = 0;
    int done = 0;

    if (!headin) {
        return NULL;
    }
    phead = (struct ts_entry *)*headin;
    if (!phead) {
        return NULL;
    }

    rootp = &phead->rlink;
    root = phead->rlink;
    if (!root) {
        return NULL;
    }
    p = root;
    while(p) {
        int kc  = compar(key,p->keyptr);
        if (!kc) {
            break;
        }
        parentp = p;
        parentcomparv = kc;
        p  = getlink(p,kc);
    }
    if (!p) {
        return NULL;
    }

    {
        /*  In Knuth Algorithm D, the parenthetical comment
            "(For example,
            if Q===RLINK(P) for some P we would set
            RLINK(P)<- LLINK(T).)"
            informs us that Q is assumed to be a conceptual name
            for some RLINK or LLINK, not a C local variable.
            In the rest of this algorithm variables can be
            ordinary C pointers, but not true for Q.
            Hence in the following we use *q to set Q. */

        struct ts_entry **q = 0;
        struct ts_entry *t  = 0;
        struct ts_entry *r  = 0;
        struct ts_entry *s  = 0;
        int emptied_a_leaf  = 0;

        /*  Either we found root (to remove) or we
            have a parentp and the parent mismatched the key so
            parentcomparv is != 0  */
        if (p == root) {
            q = rootp;
        } else if (parentcomparv < 0) {
            q = &parentp->llink;
        } else /*  (parentcomparv > 0) */ {
            q = &parentp->rlink;
        }
        /* D1.  Here *q is what Knuth calls q. */

        t = *q;
        r = t->rlink;
        if (!r) {
            *q = t->llink;
            done = 1;
        } else {
            /* D2. */
            if (!r->llink) {
                r->llink = t->llink;
                *q = r;
                done = 1;
            }
        }
        while (!done) {
            /* D3. */
            s = r->llink;
            if (s->llink) {
                r = s;
                continue;
            }
            s->llink = t->llink;
            r->llink = s->rlink;
            s->rlink = t->rlink;
            *q = s;
            done = 1;
        }
        /* Step D4. */
        if (!t->llink && !t->rlink) {
            emptied_a_leaf = 1;
        }
        free(t);

        if (emptied_a_leaf) {
            if (p == root) {
                /*  The tree is completely empty now.
                    Free the special head node.
                    Notify the caller. */
                free(phead);
                *headin = 0;
                return NULL;
            }
        }
        if (!parentp) {
            /*  The item we found was at top of tree,
                found == root.
                We have a new root node.
                We return it, there is no parent. Other than
                one might say, the fake parent phead (with only rlink,
                but that has no key so we ignore). */
            return (void *)(&(root->keyptr));
        }
        return (void *)(&(parentp->keyptr));
    }
    return NULL;
}

static void
dwarf_twalk_inner(const struct ts_entry *p,
    void (*action)(const void *nodep,
        const DW_VISIT which, const int depth),
    unsigned level)
{
    if (!p->llink && !p->rlink) {
        action((const void *)(&(p->keyptr)),dwarf_leaf,level);
        return;
    }
    action((const void *)(&(p->keyptr)),dwarf_preorder,level);
    if (p->llink) {
        dwarf_twalk_inner(p->llink,action,level+1);
    }
    action((const void *)(&(p->keyptr)),dwarf_postorder,level);
    if (p->rlink) {
        dwarf_twalk_inner(p->rlink,action,level+1);
    }
    action((const void *)(&(p->keyptr)),dwarf_endorder,level);
}

void
dwarf_twalk(const void *headin,
    void (*action)(const void *nodep, const DW_VISIT which,
        const int depth))
{
    const struct ts_entry *head = (const struct ts_entry *)headin;
    const struct ts_entry *root = 0;
    if (!head) {
        return;
    }
    root = head->rlink;
    if (!root) {
        return;
    }
    dwarf_twalk_inner(root,action,0);
}

static void
dwarf_tdestroy_inner(struct ts_entry*p,
    void (*free_node)(void *nodep),
    int depth)
{
    if (p->llink) {
        dwarf_tdestroy_inner(p->llink,free_node,depth+1);
        p->llink = 0;
    }
    if (p->rlink) {
        dwarf_tdestroy_inner(p->rlink,free_node,depth+1);
        p->rlink = 0;
    }
    /* Discards const.  Required by the interface definition. */
    free_node((void *)p->keyptr);
    free(p);
}

/*  Walk the tree, freeing all space in the tree
    and calling the user's callback function on each node.
    The user must zero out the head node, we have no
    way to do that in the defined interface.
    */
void
dwarf_tdestroy(void *headin, void (*free_node)(void *nodep))
{
    struct ts_entry *head = (struct ts_entry *)headin;
    struct ts_entry *root = 0;
    if (!head) {
        return;
    }
    root = head->rlink;
    if (head) {
        dwarf_tdestroy_inner(root,free_node,0);
    }
    free(head);
}
