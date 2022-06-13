
#ifndef _FLEXRPC_TYPES_H
#define _FLEXRPC_TYPES_H 1

typedef int bool_t;
typedef int enum_t;
/* This needs to be changed to uint32_t in the future */
typedef unsigned long flexrpcprog_t;
typedef unsigned long flexrpcvers_t;
typedef unsigned long flexrpcproc_t;
typedef unsigned long flexrpcprot_t;
typedef unsigned long flexrpcport_t;
#define        __dontcare__    -1


typedef unsigned int __uint32_t;
typedef __uint32_t uint32_t;
typedef unsigned long int __u_long;
typedef __u_long u_long;

#include <stdlib.h>                /* For malloc decl.  */
#define mem_alloc(bsize)        malloc(bsize)
#define mem_free(ptr, bsize)    free(ptr)
#ifndef TRUE
#     define TRUE   (1)
#endif

#ifndef FALSE
#     define FALSE  (0)
#endif

#ifndef NULL
#     define NULL 0
#endif

#include <stdlib.h>
#include <sys/types.h>

#endif
