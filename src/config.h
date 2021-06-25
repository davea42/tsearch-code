/* Nothing here, really, a placeholder to enable testing. */

#define HAVE_CONFIG_H 1

/* Define to 1 if tsearch is based on the AVL algorithm. */
/* #define TSEARCH_USE_BAL 1 */

/* Define to 1 if tsearch is based on the binary algorithm. */
/* #define TSEARCH_USE_BIN 1 */

/* Define to 1 if tsearch is based on the chepp algorithm. */
/* #define TSEARCH_USE_EPP 1 */

/* Define to 1 if tsearch is based on the hash algorithm. */
/* #define TSEARCH_USE_HASH 1 */
#define TSEARCH_USE_HASH 1

/* Define to 1 if tsearch is based on the red-black algorithm. */
/* #define TSEARCH_USE_RED 1 */

/* Assuming we have stdint.h and it has uintptr_t.
    Not intended to work everywhere, the tsearch
    directory stands a bit outside of libdwarf/dwarfdump.  */
#define HAVE_STDINT_H 1

/* Define 1 if we have the Windows specific header stdafx.h */
#undef HAVE_STDAFX_H

/*  Just for building tsearch independent of
    libdwarf/dwarfdump/dwarfgen. */

#define HAVE_UNUSED_ATTRIBUTE
#ifdef HAVE_UNUSED_ATTRIBUTE
#define  UNUSEDARG __attribute__ ((unused))
#else
#define  UNUSEDARG
#endif
