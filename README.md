[![Travis Build
Status](https://travis-ci.com/davea42/tsearch-code.svg?branch=master)](https://travis-ci.com/github/davea42/tsearch-code)

# This is tsearch-code README.md

Written January  2014
Updated November 2021

This is not a library.
It is a collection of implementations of
tsearch functions copyrighted by the
2-clause license ("Simplified BSD License" or "FreeBSD License")
so it's easy for anyone to copy and use in their projects that
are in C and need searchable trees at runtime.

## Building and running tests
The tests run on all five of the tsearch implementations
provided and also one test runs using libc tsearch.
For each of the six test programs there are three
test data-sets. See test/runtest.sh

If you have done git clone you need to
set things up first, then we suggest
building in a temporary directory to keep
the source tree clean:
    #Only need do this once, after clone.
    cd /path/to/tsearch-code
    sh autogen.sh

    #For any build and test, to:
    mkdir /tmp/tsbld
    cd /tmp/tsbld
    /path/to/tsearch-code/configure
    make
    #run the tests
    make check

Overall on an ordinary 3GHz 64Bit
machine a run takes around seven minutes.

## The tsearch variants


## History
I decided to implement various searches (mostly tree
searches) using the traditional UNIX/Linux tsearch
interface definitions as libdwarf needed a good
search mechanism and a more complete one,
with record deletion (dwarf_tdelete) and tree deletion
(dwarf_tdestroy) as first class functions.
Using a dwarf_ prefix to distinguish
these from any libc implementation.

The attached C code is set of five implementations of
C search code.  Based on algorithms in Knuth Volume 3
(2nd Ed) and Sedgewick 4th Edition (see more detailed
references below).

All the code here is implemented by me. I have never read the
source to any other implementations of the tsearch algorithms
or interfaces so this is a clean-room implementation.

The hash version weakens the correspondence to tree based
tsearch because, well, it's not a tree.  So twalk() behaves
differently than a tree-based version and and
has a special requirement at initialization.

Non-GNU libc usually has no tdestroy().  The set of functions
here provides a tdestroy() for all the tree/hash function
sets here.

## Why tsearch?
The interfaces are of a well-known design.  While
the interfaces are based on an obsolete style of writing
interfaces, they are a known set.  So the routines provided
here are a direct substitution.

The tdestroy() function is available in GNU libc, but is
not part of the standard tsearch() set, nor is tdestroy()
defined in the Single Unix Specification.

## Licensing
The FreeBSD open-source license (also known as
2 clause BSD) makes it easy to use this code
in any context.

## Interface oddities
For almost all users a struct (lets call it MYSTR)  has to be
malloc'd and filled in, whether for tfind, tdelete
,tsearch.

It's not really easy to understand precisely what any call did.
tfind:
 non-null returns: success, action succeeded
    The returned pointer to a MYSTR is the value.
    Free MYSTR
 null returns: Not found.
    Free MYSTR

tsearch:
 null return means an error occurred.
    Free MYSTR
 non-null return 
    if the returnval == MYSTR:
      tsearch success - added record
      So do not free MYSTR!
    else
      tsearch success - record already in tree
      so nothing added or changed.
      Free MYSTR.

tdelete:
 If the passed-in pointer to the root pointer
   now points to a NULL value the tree is now empty.
   This information is of marginal value.
 non-null return  
    success, action succeeded
    The return value is not very useful, its content
      is not easily described.
    Free MYSTR
 Null return:
    Fail due to internal error or improper argument.
    Or it means the delete suceeded and the tree
    is now empty.
    free MYSTR

When using the hash search, tdelete return of Null
does not necessarily mean the tree (well, we
just call it that even for hashing) is empty, and
the passed-in pointer to the root pointer
does not point to NULL.

Speaking of C here, so, there is no try/catch available.

It might have been nice if twalk() let the user-provided
action code indicate to twalk() that the user wanted it to stop
walking the tree.

We are staying with the standard interfaces.

## Implementation oddities
The code uses names like dwarf_tsearch() instead of just
tsearch() so one can have both this tsearch() and GNU or
a standard tsearch code available simultaneously with no
interference at runtime.   The code here operates like GNU or
UNIX tsearch() but is internally incompatible.  We'll usually
refer to tsearch() not dwarf_tsearch() (etc) but we mean either
and both unless otherwise specified here.

The use of tsearch() and friends is a little tricky.
That is just the way it is.  The best reference is the code
in tsearch_tester.c, but see above for a plain text
description.

The hash version of tdelete() cannot always return a non-null
value even if there are  records left.  Not being a tree at
all, it would cost runtime to provide a non-null return in
some cases even when there are records left from tdelete().
The return value from tdelete() is problematic in all versions,
since it purports to return the parent of the deleted node,
but what if the deleted node was the root?

Note the trivial set of #defines in tsearch_tester.c that
result in a standard-based naming of the functions so
the libc version can be tested.

The return value from tdelete() is particularly problematic
because it purports to return a pointer to another record
than the one deleted, yet no such record necessarily exists,
but if something is deleted a non-null pointer is returned.

The hash version requires use of the function
dwarf_initialize_search_hash() to provide a hashing
function. And the hash version does not set the root pointer
to NULL on tdelete() of the last record, since that would
result in losing the hashing function pointer.  Use tdestroy()
to free up any remaining space. The other versions implement
that function and the function does nothing for those.

dwarf_tdump() is an invention and unique to this code.  It is
for debugging. It prints a representation of the data in the
tree or hash to stdout.

## Other directories
test contains a set of sample allocations, where
lines starting with 'a' mean add and 'd' means delete.
These are used for testing the library code.

scripts contains some python scripts for converting
results produced by printf added to libdwarf alloca()
code.  These were used massage the print output
from running dwarfdump on real object files
into the testcase/* format.
It is unlikely you will find them much use
except as starting points.

Donald E Knuth, The Art of Computer Programming
Volume 3, Second Edition. Section 6.2.2 Binary
Tree Searching, page 426.  Chapter 6.2.3 Balanced
Trees, page 458.

Robert Sedgewick and Kevin Wayne, Algorithms (4th
Ed).  This is the crucial reference on red-black
trees.  There is a crucial fix in the errata of the
3rd printing.  In addition, in dwarf_tsearchred.c in
two places I had to fix things, look for 'Mistake'
in the source.

http://www.prevanders.net/tsearch.html
