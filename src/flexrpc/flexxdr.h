
#ifndef _FLEX_XDR_H
#define _FLEX_XDR_H

#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "types.h"

__BEGIN_DECLS

enum flexxdr_op {
  FLEXXDR_ENCODE = 0,
  FLEXXDR_DECODE = 1,
  FLEXXDR_FREE = 2
};

typedef struct FLEXXDR FLEXXDR;

/*
 * This is the numer of bytes per unit of external data
 */
#define BYTES_PER_FLEXXDR_UNIT      (4)

#if 1
#define RNDUP(x) (((x) - BYTES_PER_FLEXXDR_UNIT - 1) & ~(BYTES_PER_FLEXXDR_UNIT - 1))
#else
#define RNDUP(x)  ((((x) + BYTES_PER_FLEXXDR_UNIT - 1) / BYTES_PER_FLEXXDR_UNIT) \ * BYTES_PER_FLEXXDR_UNIT)
#endif

struct FLEXXDR
{
  enum flexxdr_op f_op;
  struct flexxdr_ops
  {
    bool_t (*f_getlong) (FLEXXDR *__flexxdrs, long *__lg);
    bool_t (*f_putlong) (FLEXXDR *__flexxdrs, const long *__lp);
    bool_t (*f_getbytes) (FLEXXDR *__flexxdrs, char * __addr, u_int __len);
    bool_t (*f_putbytes) (FLEXXDR *__flexxdrs, const char * __addr, u_int __len);
    u_int (*f_getpostn) (const FLEXXDR *__flexxdrs);
    /* returns bytes off from beginning */
    bool_t (*f_setpostn) (FLEXXDR *__flexxdrs, u_int __pos);
    int32_t *(*f_inline) (FLEXXDR *__flexxdrs, u_int __len);
    void (*f_destroy) (FLEXXDR *__flexxdrs);

  }
  *f_ops;

  char * f_public;
  char * f_private;
  char * f_base;
  u_int f_handy;
};


/* A flexxdrproc_t exisit for each data type which is to be encoded or decoded
 *
 * The second argument to the flexxdrproc_t is a pointer to an opaque pointer.
 * The opaque pointer generally points to a structure of the data type to be decoded. If this pointer is 0, then the type routines shoudl allocate dynamic 
 * storage of the appropriate size and return it.
 */
typedef bool_t (*flexxdrproc_t)(FLEXXDR *, char *, ...);

/*
 * Operations defined on a FLEXXDR handle
 *
 * FLEXXDR    *flexxdr;
 * long       *longp;
 * void*        addr;
 * u_int        len;
 * u_int        pos;
 */
#define FLEXXDR_GETLONG(flexxdrs, longp)                \
        (*(flexxdrs)->f_ops->f_getlong)(flexxdrs, longp)

#define flexxdr_getlong(flexxdrs, longp)                \
        (*(flexxdrs)->f_ops->f_getlong)(flexxdrs, longp)

#define FLEXXDR_PUTLONG(flexxdrs, longp)                \
        (*(flexxdrs)->f_ops->f_putlong)(flexxdrs, longp)

#define flexxdr_putlong(flexxdrs, longp)                \
        (*(flexxdrs)->f_ops->f_putlong)(flexxdrs, longp)
#define flexxdr_getbytes(flexxdrs, addr, len)           \
        (*(flexxdrs)->f_ops->f_getbytes)(flexxdrs, addr, len)
#define FLEXXDR_GETBYTES(flexxdrs, addr, len)           \
        (*(flexxdrs)->f_ops->f_getbytes)(flexxdrs, addr, len)
#define flexxdr_putbytes(flexxdrs, addr, len)           \
        (*(flexxdrs)->f_ops->f_putbytes)(flexxdrs, addr, len)
#define FLEXXDR_PUTBYTES(flexxdrs, addr, len)           \
        (*(flexxdrs)->f_ops->f_putbytes)(flexxdrs, addr, len)
#define FLEXXDR_GETPOS(flexxdrs)                                \
        (*(flexxdrs)->f_ops->f_getpostn)(flexxdrs)
#define flexxdr_getpos(flexxdrs)                                \
        (*(flexxdrs)->f_ops->f_getpostn)(flexxdrs)
#define FLEXXDR_SETPOS(flexxdrs, pos)                                \
        (*(flexxdrs)->f_ops->f_setpostn)(flexxdrs, pos)
#define flexxdr_setpos(flexxdrs, pos)                                \
        (*(flexxdrs)->f_ops->f_setpostn)(flexxdrs, pos)
#define FLEXXDR_INLINE(flexxdrs, len)                                \
        (*(flexxdrs)->f_ops->f_inline)(flexxdrs, len)
#define flexxdr_inline(flexxdrs, len)                                \
        (*(flexxdrs)->f_ops->f_inline)(flexxdrs, len)

/*
 * Inline routines for fast encode/decode of primitive data types.
 * Caveat emptor: these use single memory cycles to get the
 * data from the underlying buffer, and will fail to operate
 * properly if the data is not aligned.  The standard way to use these
 * is to say:
 *      if ((buf = FLEXXDR_INLINE(xdrs, count)) == NULL)
 *              return (FALSE);
 *      <<< macro calls >>>
 * where ``count'' is the number of bytes of data occupied
 * by the primitive data types.
 *
 * N.B. and frozen for all time: each data type here uses 4 bytes
 * of external representation.
 */
#define IFLEXXDR_GET_INT32(buf)           ((int32_t)ntohl((uint32_t)*(buf)++))
#define IFLEXXDR_PUT_INT32(buf, v)        (*(buf)++ = (int32_t)htonl((uint32_t)(v)))
#define IFLEXXDR_GET_U_INT32(buf)         ((uint32_t)IFLEXXDR_GET_INT32(buf))
#define IFLEXXDR_PUT_U_INT32(buf, v)      IFLEXXDR_PUT_INT32(buf, (int32_t)(v))
/* WARNING: The IFLEXXDR_*_LONG defines are removed by Sun for new platforms
 * and shouldn't be used any longer. Code which use this defines or longs
 * in the RPC code will not work on 64bit Solaris platforms !
 */
#define IFLEXXDR_GET_LONG(buf) ((long)IFLEXXDR_GET_U_INT32(buf))
#define IFLEXXDR_PUT_LONG(buf, v) ((long)IFLEXXDR_PUT_INT32(buf, (long)(v)))
#define IFLEXXDR_GET_U_LONG(buf)              ((u_long)IFLEXXDR_GET_LONG(buf))
#define IFLEXXDR_PUT_U_LONG(buf, v)              IFLEXXDR_PUT_LONG(buf, (long)(v))
#define IFLEXXDR_GET_BOOL(buf)            ((bool_t)IFLEXXDR_GET_LONG(buf))
#define IFLEXXDR_GET_ENUM(buf, t)         ((t)IFLEXXDR_GET_LONG(buf))
#define IFLEXXDR_GET_SHORT(buf)           ((short)IFLEXXDR_GET_LONG(buf))
#define IFLEXXDR_GET_U_SHORT(buf)         ((u_short)IFLEXXDR_GET_LONG(buf))
#define IFLEXXDR_PUT_BOOL(buf, v)         IFLEXXDR_PUT_LONG(buf, (long)(v))
#define IFLEXXDR_PUT_ENUM(buf, v)         IFLEXXDR_PUT_LONG(buf, (long)(v))
#define IFLEXXDR_PUT_SHORT(buf, v)        IFLEXXDR_PUT_LONG(buf, (long)(v))
#define IFLEXXDR_PUT_U_SHORT(buf, v)      IFLEXXDR_PUT_LONG(buf, (long)(v))


#define NULL_flexxdrproc_t ((flexxdrproc_t)0)
struct flexxdr_discrim
{
  int value;
  flexxdrproc_t proc;
};

// MACROS


// Generic routings
extern bool_t flexxdr_void (void) __THROW;
extern bool_t flexxdr_long (FLEXXDR * __flexxdrs, long *__lp) __THROW;
extern bool_t flexxdr_u_long (FLEXXDR * __flexxdrs, u_long *__ulp) __THROW;
extern bool_t flexxdr_int(FLEXXDR *__flexxdr, int *__ip) __THROW;
extern bool_t flexxdr_enum(FLEXXDR *__flexxdrs, enum_t *__ep) __THROW;
extern bool_t flexxdr_union(FLEXXDR *__flexxdrs, enum_t *__dscmp, char *__unp, const struct flexxdr_discrim *__choices, flexxdrproc_t __dfault) __THROW;
extern bool_t flexxdr_bytes(FLEXXDR *__flexxdrs, char **__cpp, u_int *__sizep, u_int __maxsize) __THROW;
extern bool_t flexxdr_string (FLEXXDR *__xdrs, char **__cpp, u_int __maxsize) __THROW;
extern bool_t flexxdr_char (FLEXXDR *__flexxdrs, char *__cp) __THROW;
extern bool_t flexxdr_u_int (FLEXXDR *__flexxdrs, u_int *__up) __THROW;
extern bool_t flexxdr_opaque (FLEXXDR *__flexxdrs, char * __cp, u_int __cnt) __THROW;

extern void flexxdrmem_create(FLEXXDR *__flexxdrs, const char* __addr, u_int __size, enum flexxdr_op __fop) __THROW;

extern void flexxdr_create(FLEXXDR *__flexxdr, u_int __sendsize,
    u_int __recvsize, char * __flextcp_handle, 
    int (*__readit)(char *, char *, int),
    int (*__writeit)(char *, char *, int)) __THROW;

extern void flexxdrrec_create( FLEXXDR *__flexxdrs, u_int __sendsize, u_int __recvsize, char * __flextcp_handle, int (*__readit)(char *, char *, int), int (*__writeit)(char *, char *, int)) __THROW;

extern bool_t flexxdrrec_endofrecord (FLEXXDR *__flexxdrs, bool_t __sendnow) __THROW;

extern bool_t flexxdrrec_skiprecord (FLEXXDR *__flexxdrs) __THROW;
typedef struct rec_strm
{
  char * flextcp_handle;
  char * flex_buffer;

  /* out-going bits */
  int (*writeit) (char *, char *, int);   /* output buffer */
  char * out_base;     /* output buffer */
  char * out_finger;  /* next output position */ 
  char * out_boundary;  /*data cannot up to this address */
  uint32_t *frag_header;  /* beginning of current fragment */ 
  bool_t frag_sent; /* true if buffefr sent in middle of record */

  /* incoming bits */
  int (*readit)(char *, char *, int);   /* input buffer */
  u_long in_size;
  char * in_base;
  char * in_finger;         /* location of next byte ot be had */
  char * in_boundary;       /* can read up to this location */
  long fbtbc;               /* fragment bytes to be consumed */
  bool_t last_frag;
  u_int sendsize;
  u_int recvsize;
} RECSTREAM;

__END_DECLS
#endif
