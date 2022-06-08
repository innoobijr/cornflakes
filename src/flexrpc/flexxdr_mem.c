#include <string.h>
#include <limits.h>
#include <arpa/inet.h>

#include "flexxdr.h"
#include "types.h"

static bool_t flexxdrmem_getlong (FLEXXDR *, long *);
static bool_t flexxdrmem_putlong (FLEXXDR *, const long *);
static bool_t flexxdrmem_getbytes (FLEXXDR *, char *, u_int);
static bool_t flexxdrmem_putbytes (FLEXXDR *, const char *, u_int);
static u_int flexxdrmem_getpos (const FLEXXDR *);
static bool_t flexxdrmem_setpos (FLEXXDR *, u_int);
static int32_t *flexxdrmem_inline (FLEXXDR *, u_int);

static const struct flexxdr_ops flexxdrmem_ops = 
{
  flexxdrmem_getlong,
  flexxdrmem_putlong,
  flexxdrmem_getbytes,
  flexxdrmem_putbytes,
  flexxdrmem_getpos,
  flexxdrmem_setpos,
  flexxdrmem_inline
};

/* The procedure flexxdrmem_create initializaien a stream descriptor for a memory buffer 
 */
void
flexxdrmem_create(FLEXXDR *flexxdrs, const char * addr, u_int size, enum flexxdr_op op)
{
  flexxdrs->f_op = op;
  flexxdrs->f_ops = (struct flexxdr_ops *) &flexxdrmem_ops;
  flexxdrs->f_private = flexxdrs->f_base = (char *) addr;
  flexxdrs->f_handy = size;
}

void
flexxdrmem_destroy(FLEXXDR *flexxdr)
{
}

static bool_t
flexxdrmem_getlong (FLEXXDR *flexxdrs, long *lp)
{
  if (flexxdrs->f_handy < 4)
    return FALSE;
  flexxdrs->f_handy -= 4;
  *lp = (int32_t) ntohl ((*((int32_t *) (flexxdrs->f_private))));
  flexxdrs->f_private += 4;
  return TRUE;
}

static bool_t
flexxdrmem_putlong (FLEXXDR *flexxdrs, const long *lp)
{
  if (flexxdrs->f_handy < 4)
    return FALSE;
  flexxdrs->f_handy -= 4;
  *(int32_t *) flexxdrs->f_private = htonl (*lp);
  flexxdrs->f_private += 4;
  return TRUE;
}

static bool_t
flexxdrmem_getbytes (FLEXXDR *flexxdrs, char * addr, u_int len)
{
  if (flexxdrs->f_handy < len)
    return FALSE;
  flexxdrs->f_handy -= len;
  memcpy(addr, flexxdrs->f_private, len);
  flexxdrs->f_private += len;
  return TRUE;
}

static bool_t
flexxdrmem_putbytes (FLEXXDR *flexxdrs, const char *addr, u_int len)
{
  printf("_putbyptes: %s\n", addr);
  printf("_putbyptes: processing\n");
  if (flexxdrs->f_handy < len)
    return FALSE;
  printf("_putbyptes: handy-pos: [%d]\n", flexxdrs->f_handy);
  flexxdrs->f_handy -= len;
  printf("_putbyptes: processing. Doing copy\n");
  memmove(flexxdrs->f_private, addr, len);
  printf("_putbyptes: copy successful\n");
  flexxdrs->f_private += len;
  printf("_putbyptes: returning from copy\n");
  return TRUE;
}

static u_int
flexxdrmem_getpos(const FLEXXDR *flexxdrs)
{
  return (u_long) flexxdrs->f_private - (u_long) flexxdrs->f_base;
}

static bool_t
flexxdrmem_setpos(FLEXXDR *flexxdrs, u_int pos)
{
  char * newaddr = flexxdrs->f_base + pos;
  char * lastaddr = flexxdrs->f_private - flexxdrs->f_handy;
  size_t handy = lastaddr - newaddr;

  if (newaddr > lastaddr
      || newaddr < flexxdrs->f_base
      || handy != (u_int) handy) return FALSE;

  flexxdrs->f_private = newaddr;
  flexxdrs->f_handy = (u_int) handy;
  return TRUE;
}

static int32_t *
flexxdrmem_inline (FLEXXDR *flexxdrs, u_int len)
{
  int32_t *buf = 0;
  printf("Handy  is: [%d] [%d]\n", flexxdrs->f_handy, len);

  if (flexxdrs->f_handy >= len)
  {
    flexxdrs->f_handy -= len;
    buf = (int32_t *) flexxdrs->f_private;
    flexxdrs->f_private += len;
  }
  return buf;
}

