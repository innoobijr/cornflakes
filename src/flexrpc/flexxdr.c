#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <libintl.h>
#include <wchar.h>
#include <stdint.h>

#include "types.h"
#include "flexrpc.h"

#define FLEXXDR_FALSE        ((long) 0)
#define FLEXXDR_TRUE        ((long) 1)
#define LASTUNSIGNED        ((u_int) 0-1)


static const char flexxdr_zero[BYTES_PER_FLEXXDR_UNIT] = {0,0,0,0};

bool_t
flexxdr_void (void)
{
  return TRUE;
}

bool_t
flexxdr_int(FLEXXDR *flexxdrs, int *ip)
{
  long l;

  switch (flexxdrs->f_op)
  {
    case FLEXXDR_ENCODE:
      l = (long) *ip;
      printf("putting something\n");
      return FLEXXDR_PUTLONG (flexxdrs, &l);

    case FLEXXDR_DECODE:
      printf("Number is: [%ld]\n", (long) *ip);

      if (!FLEXXDR_GETLONG (flexxdrs, &l))
      {
        printf("Returning false\n");
        return FALSE;
      }
      printf("Number is: [%d]\n", *((u_int *) ip));

      *ip = (int) l;
    case FLEXXDR_FREE:
      return TRUE;
  }
  return FALSE;
}


bool_t
flexxdr_enum(FLEXXDR *flexxdrs, enum_t *ep)
{
  enum sizecheck
  {
    SIZEVAL
  };

  /*
   * enums are treated as ints
   */
  {
    long l;

    switch (flexxdrs->f_op)
    {
      case FLEXXDR_ENCODE:
        l = *ep;
        return FLEXXDR_PUTLONG (flexxdrs, &l);
      case FLEXXDR_DECODE:
        if (! FLEXXDR_GETLONG (flexxdrs, &l))
        { 
          return FALSE;
        }
        *ep = l;
        /* Fall through */
      case FLEXXDR_FREE:
        return TRUE;

    }
    return FALSE;
  }
}

bool_t
flexxdr_union (FLEXXDR *flexxdrs,
                /* enum ot decide which arm to work on*/
                enum_t *dscmp,
                /* the union itself */
                char *unp,
                /* [value, xdr_proc] for each arm */
                const struct flexxdr_discrim *choices,
                /* default xdr routing */
                flexxdrproc_t dfault)
{
  enum_t dscm;

  /* Deal with the discriminator: its an enum */
  if (!flexxdr_enum(flexxdrs, dscmp))
  { 
      return FALSE;
  }
  dscm = *dscmp;

  /* search choices for a value that matches the discriminator. if we find one, execute the xdr routing for that value.*/
  for (; choices->proc != NULL_flexxdrproc_t; choices++)
  {
    if (choices->value == dscm)
      return (*(choices->proc))(flexxdrs, unp, LASTUNSIGNED);
  }

  /* No match -  execute the default xdr routing */
  return ((dfault == NULL_flexxdrproc_t) ? FALSE :
      (*dfault) (flexxdrs, unp, LASTUNSIGNED));
}


bool_t
flexxdr_long(FLEXXDR *flexxdrs, long *lp)
{
  if (flexxdrs->f_op == FLEXXDR_ENCODE
      && (sizeof (int32_t) == sizeof (long)
        || (int32_t) *lp == *lp))
    return FLEXXDR_PUTLONG (flexxdrs, lp);

  if (flexxdrs->f_op == FLEXXDR_DECODE)
    return FLEXXDR_GETLONG (flexxdrs, lp);

  return FALSE;
}

bool_t
flexxdr_u_long(FLEXXDR *flexxdrs, u_long *ulp)
{
  switch (flexxdrs->f_op)
  {
    case FLEXXDR_DECODE:
      {
        long int tmp;
        if (FLEXXDR_GETLONG (flexxdrs, &tmp) == FALSE)
          return FALSE;

        *ulp = (uint32_t) tmp;
        return TRUE;
      }

    case FLEXXDR_ENCODE:
      if (sizeof(uint32_t) != sizeof (u_long)
          && (uint32_t) *ulp != *ulp)
        return FALSE;

      return FLEXXDR_PUTLONG (flexxdrs, (long *) ulp);

    case FLEXXDR_FREE:
      return TRUE;
  }
  return FALSE;
}

bool_t
flexxdr_char(FLEXXDR *flexxdrs, char *cp)
{
  int i;
  i =  (*cp);
  if (!flexxdr_int (flexxdrs, &i))
  {
    return FALSE;
  }
  *cp = i;
  return TRUE;
}

bool_t
flexxdr_u_int (FLEXXDR *flexxdrs, u_int *up) 
{
  long l;

  switch (flexxdrs->f_op)
  {
    case FLEXXDR_ENCODE:
      printf("flexxdr_u_int: [%d]", *up),

      l = (u_long) * up;
      bool_t o = FLEXXDR_PUTLONG(flexxdrs, &l);
      printf("Seg #4\n");
      return o;
    case FLEXXDR_DECODE:
      if (!FLEXXDR_GETLONG (flexxdrs, &l))
      { return FALSE; }
      *up = (u_int) (u_long) l;
      /* Fall through */
    case FLEXXDR_FREE:
      return TRUE;
  }
  printf("Seg #3\n");
  return FALSE;
}

bool_t
flexxdr_opaque( FLEXXDR *flexxdrs, char * cp, u_int cnt)
{
  u_int rndup;
  static char crud[BYTES_PER_FLEXXDR_UNIT];

  printf("flexxdr_opaque: checking string input [%s]\n", (char *) cp);

  /*
   * if no data then we are done
   */
  if (cnt == 0)
    return TRUE;

  /* 
   * round byte coutn to fill xdr units
   */
  rndup = cnt % BYTES_PER_FLEXXDR_UNIT;
  if (rndup > 0)
    rndup = BYTES_PER_FLEXXDR_UNIT - rndup;
  printf("flexxdr_opaque: calcualted rndbytes [rndup:%d, cnt:%d, bytes:%d]\n", rndup, cnt, BYTES_PER_FLEXXDR_UNIT);

  switch(flexxdrs->f_op)
  {
    case FLEXXDR_DECODE:
      printf("flexxdr_opaque: decoding structre \n");
      if (!FLEXXDR_GETBYTES(flexxdrs, cp, cnt))
      {
        return FALSE;
      }
      printf("Grabdded bytes\n");

      if (rndup == 0)
        return TRUE;
      return FLEXXDR_GETBYTES(flexxdrs, (char *) crud, rndup);

    case FLEXXDR_ENCODE:
      printf("flexxdr_opaque: setting encoding [cp:%s, cnt:%d]\n", (char *)cp, cnt);

      if (!FLEXXDR_PUTBYTES (flexxdrs, (char *) cp, cnt))
      { 
        return FALSE; 
      }
      printf("flexxdr_opaque: putbytes successful [cp: %s]\n", (char *) cp);

       printf("flexxdr_opaque: round up size is %d\n", rndup);
        printf("flexxdr_opaque: setting encoding [cp:%s,]\n", (char *) cp);

      if (rndup == 0)
        return TRUE;
      return FLEXXDR_PUTBYTES(flexxdrs, flexxdr_zero, rndup);

    case FLEXXDR_FREE:
      return TRUE;
  }
  return FALSE;
}
/* FLEXXDR counted bytes
 * *cpp is a pointer to the bytes , *sizep is the count. If *cpp is NuLL maxsize bytes
 * are allocated
 */
bool_t
flexxdr_bytes(FLEXXDR *flexxdrs, char **cpp, u_int *sizep, u_int maxsize)
{
  char *sp = *cpp; /* sp is the actualy string pointer */
  u_int nodesize;
  printf("_bytes: processing\n");
  /*
   * first deal wit teh length sinec xdr bytes are counters */
  if (!flexxdr_u_int(flexxdrs, sizep))
  { 
    return FALSE;
  }

  printf("_bytes: nodesize succcessful\n");

  nodesize = *sizep;
  if ((nodesize > maxsize) && (flexxdrs->f_op != FLEXXDR_FREE))
  { 
    return FALSE;
  }

  /*
   * now deal with teh actuall bytes
   */
  printf("_bytes: nodesize set\n :: %d", nodesize);

  switch(flexxdrs->f_op)
  {
    case FLEXXDR_DECODE:
      if (nodesize == 0) { return TRUE; }
      if (sp == NULL) { 
        *cpp = sp = (char *) mem_alloc(nodesize);
      }
      if (sp == NULL) 
      {
        //(void) fprintf(NULL, "%s: %s", __func__, _("out of memory\n"));
        return FALSE;
      }
      /* Fall throught **/
    case FLEXXDR_ENCODE:
      printf("_bytes: running FLEXXDR_ENCODE\n");
      return flexxdr_opaque(flexxdrs, sp, nodesize);

    case FLEXXDR_FREE:
      if (sp != NULL)
      {
        // Free memory here
      }
      return TRUE;
  }
  return FALSE;
}

bool_t
flexxdr_string(FLEXXDR *flexxdrs, char **cpp, u_int maxsize)
{
  char *sp = *cpp;
  u_int size = 0;
  u_int nodesize;
  bool_t ret;

  /*
   * first deal with the length since xdr strings are counted-strings
   */
  switch(flexxdrs->f_op) 
  {
    case FLEXXDR_FREE:
      if (sp == NULL)
      { 
        return TRUE; 
      }
    case FLEXXDR_ENCODE:
      if (sp == NULL)
        return FALSE;
      size = strlen(sp);
      break;
    case FLEXXDR_DECODE:
        break;
  }

  printf("flexxdr_string: [ size: %d\n]", size);

  if (!flexxdr_u_int(flexxdrs, &size)){
    printf("flexxdr_string: flexxdr_u_int fails\n");
    return FALSE;
  }

  printf("flexxdr_string: reset-- [ size: %d\n]", size);

  if (size > maxsize) { return FALSE; }
  
  nodesize = size + 1;
  if (nodesize == 0) {
    /* This means that there is an overflows. IT is a bug in teh caller which provided too large maxsize
     */
    return FALSE;
  }

  printf("Still alive\n");

  /* Deal with actual bytes */

  switch(flexxdrs->f_op) {
    case FLEXXDR_DECODE:
      printf("In decode\n");
      if (sp == NULL) {
        printf("allocaitng space\n");
        *cpp = sp = mem_alloc(nodesize);
      }

      if (sp == NULL) {
        printf("xdr_string: out of memory\n");
        return FALSE;
      }
      printf("about to blow up [%s]\n", sp);
      //sp[size] = 0;
      //memset(sp,0,size);
      /* DECODE falls through */

    case FLEXXDR_ENCODE:
      printf("Faliing through\n");
      ret = flexxdr_opaque(flexxdrs, sp, size);
      printf("flexxdr_string: encoding\n");
      return ret;

      /*if ((flexxdrs->f_op == FLEXXDR_DECODE)  && (ret == FALSE)) {
        if (allocated == TRUE) {
          printf("flexxdr_string: freeing up string\n");
          free(sp);
          *cpp = NULL;
        }
      }*/

    case FLEXXDR_FREE:
      // implement free
      return TRUE;
  }
  return FALSE;
}
