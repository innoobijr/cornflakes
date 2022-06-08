/*
 * flexxdr_rec.c, implement a FLEX TCP/IP based XDR stream with record marking
 *
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <signal.h>

#include "flexxdr.h"

static bool_t flexxdrrec_getlong(FLEXXDR *, long *);
static bool_t flexxdrrec_putlong(FLEXXDR *, const long *);
static bool_t flexxdrrec_getbytes(FLEXXDR *, char *, u_int);
static bool_t flexxdrrec_putbytes(FLEXXDR *, const char *, u_int);

static const struct flexxdr_ops flexxdrrec_ops = {
  flexxdrrec_getlong,
  flexxdrrec_putlong,
  flexxdrrec_getbytes,
  flexxdrrec_putbytes
};


/* A recrod is composed of one or more record fragments.
 * A record framgment is two-byte header followed by zero to 2**32-1 bytes. The header is teated as a long unsigned and is encoded/decoded to the network via htonl/ntonl. The low order 31 bits are a byte count of the fragment. The highest order bit is a boolean: 1 => fragment is the last fragment of the record,
 * 0 => this fragment is followed by more fragment(s).
 *
 * The fragment/record machinery is not genera; it is constrcuted t meet the needs of xdr and rpc based ontop of flextcp.
 */

#define LAST_FRAG (1UL << 31)

static u_int fix_buf_size (u_int);
static bool_t skip_input_bytes(RECSTREAM *, long);
static bool_t flush_out (RECSTREAM *, bool_t);
static bool_t set_input_fragment(RECSTREAM *);
static bool_t get_input_bytes (RECSTREAM *, char *, int);

/* 
 * Create an xdr handle for flexxdrrec
 * flexxdrrec_create fills in flexxdrs. Sendsize and recvsize are send and recv buffer size (0 => use default).
 * tcp_handle is an opaque handle tha stis passed as the first parameter to the procedures readits adn writeit. They are read adn write respectively. They arelike systems calls excepts they take an opaque handle rathern than an fd
 */
void 
flexxdrrec_create(FLEXXDR *flexxdrs, u_int sendsize, u_int recvsize, char * flextcp_handle, int (*readit)(char *, char *, int),
    int (*writeit)(char *, char *, int))
{
  RECSTREAM *rstrm = (RECSTREAM *) mem_alloc (sizeof (RECSTREAM));
  char * tmp;
  char *buf;

  sendsize = fix_buf_size(sendsize);
  recvsize = fix_buf_size(recvsize);
  buf = mem_alloc(sendsize + recvsize + BYTES_PER_FLEXXDR_UNIT);

  if (rstrm == NULL ||  buf == NULL)
  { 
    printf("Out of memory error/n");
    mem_free(rstrm, sizeof(RECSTREAM));
    mem_free(buf, sendsize + recvsize + BYTES_PER_FLEXXDR_UNIT);
    return;
  }

  /* adjust sizes and allcoated buffer quat bytes aligned */

  rstrm->sendsize = sendsize;
  rstrm->recvsize = recvsize;
  rstrm->flex_buffer = buf;
  tmp = rstrm->flex_buffer;
  if ((size_t)tmp % BYTES_PER_FLEXXDR_UNIT)
    tmp += BYTES_PER_FLEXXDR_UNIT - (size_t) tmp % BYTES_PER_FLEXXDR_UNIT;
  rstrm->out_base = tmp;
  rstrm->in_base = tmp + sendsize;
  /*
   * now the rest...
   * We have to add the cast since the `struct xdr_ops` in struct XDR is not a const. */
  flexxdrs->f_ops = (struct flexxdr_ops *) &flexxdrrec_ops;
  flexxdrs->f_private = (char *) rstrm;
  rstrm->flextcp_handle = flextcp_handle;
  rstrm->readit = readit;
  rstrm->writeit = writeit;
  rstrm->out_finger = rstrm->out_boundary = rstrm->out_base;
 printf("out_finger [%p], out_boundary [%p], out_base [%p]\n", rstrm->out_finger, rstrm->out_boundary, rstrm->out_base);
  rstrm->frag_header = (uint32_t *) rstrm->out_base;
  rstrm->out_finger += 16;
  rstrm->out_boundary = rstrm->out_finger + sendsize;
  rstrm->frag_sent = FALSE;
  rstrm->in_size = recvsize;
  rstrm->in_boundary = rstrm->in_base;
  rstrm->in_finger = (rstrm->in_boundary += recvsize);
  rstrm->fbtbc = 0;
  rstrm->last_frag = TRUE;

  printf("out_finger [%p], out_boundary [%p], out_base [%p]\n", rstrm->out_finger, rstrm->out_boundary, rstrm->out_base);

}

/* 
 * The routines defined below are the flexxdr ops which will go into the flexxdr handle filled in by flexxdrrec_create.
 */
static bool_t
flexxdrrec_getlong(FLEXXDR *flexxdrs, long *lp)
{
  printf("Getting something 1[%ld]\n", *lp);

  RECSTREAM *rstrm = (RECSTREAM *) flexxdrs->f_private;
  int32_t *buflp = (int32_t *) rstrm->in_finger;
  int32_t mylong;

  /* first try the iline, fast case */
  if (rstrm->fbtbc >= BYTES_PER_FLEXXDR_UNIT && (char *) rstrm->in_boundary - (char *) buflp >= BYTES_PER_FLEXXDR_UNIT) {
    *lp = (int32_t) ntohl (*buflp);
    printf("Getting something 2[%ld]\n", *lp);
    rstrm->fbtbc -= BYTES_PER_FLEXXDR_UNIT;
    rstrm->in_finger += BYTES_PER_FLEXXDR_UNIT;
  }
  else
  {
    printf("flexprint\n");
    if (!flexxdrrec_getbytes (flexxdrs, (char *) &mylong, BYTES_PER_FLEXXDR_UNIT)){
      printf("fail\n");
      return FALSE;
    }
    *lp = (int32_t) ntohl (mylong);
    printf("Getting something 3 [%ld]\n", *lp);
  }
  return TRUE;
}

static bool_t
flexxdrrec_putlong(FLEXXDR *flexxdrs, const long *lp)
{

  printf("Putting something here\n");

  RECSTREAM *rstrm = (RECSTREAM *) flexxdrs->f_private;
  int32_t *dest_lp = (int32_t *) rstrm->out_finger;
  printf("Seg #1: [%p],[[%p], %p]\n", rstrm->out_boundary, rstrm->out_finger - 16, rstrm->out_finger + BYTES_PER_FLEXXDR_UNIT);
  if ((rstrm->out_finger += BYTES_PER_FLEXXDR_UNIT) > rstrm->out_boundary)
  {
    /*
     * this case should almost never happen so the code is inefficient */
    rstrm->out_finger -= BYTES_PER_FLEXXDR_UNIT;
    rstrm->frag_sent = TRUE;
    printf("Edge case\n");
    if (!flush_out (rstrm, FALSE))
      return FALSE;
    printf("Edge case out\n");
    dest_lp = (int32_t *) rstrm->out_finger;
    rstrm->out_finger += BYTES_PER_FLEXXDR_UNIT;
  }
  printf("Adding value\n");
  *dest_lp = htonl (*lp);
  rstrm->out_finger = (char *) dest_lp;
  printf("Segggg: [%d]\n", ntohl(*((int32_t *)rstrm->out_finger)));
  return TRUE;
}

static bool_t
flexxdrrec_getbytes(FLEXXDR *flexxdrs, char * addr, u_int len)
{
  RECSTREAM *rstrm = (RECSTREAM *) flexxdrs->f_private;
  u_int current;

  while (len > 0)
  {
    current = rstrm->fbtbc;
    printf("Current is 12 [%d][%d]\n", current, len);
    if (current == 0)
    {
      printf("Cirrent fail\n");
      if (rstrm->last_frag){
        printf("Last fragment\n");
        return FALSE;
      }
      printf("set_input_fragment: start \n");
      if (!set_input_fragment (rstrm)){
        printf("Setting input failed\n");
        return FALSE;
      }
      printf("Setting success, continuing\n");
      continue;
    }

    printf("fail #4\n");
    current = (len < current ) ? len : current;
    if (!get_input_bytes(rstrm, addr, current))
      return FALSE;
    addr += current;
    rstrm->fbtbc -= current;
    len -= current;
  }
  return TRUE;
}

static bool_t
flush_out (RECSTREAM *rstrm, bool_t eor)
{
  printf("Header: status %ld\n", LAST_FRAG);
  u_long eormask = (eor == TRUE) ? LAST_FRAG : 0;
  printf("eormask %s\n", (char *) rstrm->frag_header);
  u_long len = ((int32_t *)rstrm->out_finger - (int32_t *) rstrm->frag_header - BYTES_PER_FLEXXDR_UNIT);
  
  printf("Length: %d\n", ntohl(*((int32_t *)rstrm->out_finger)));

  *rstrm->frag_header = htonl (len | eormask);
  printf("Seg #10\n");
  //len = rstrm->out_finger - rstrm->out_base;
  len = sizeof(int32_t);

  printf("Output length is %ld\n", len);
  if ((*(rstrm->writeit)) (rstrm->flextcp_handle, rstrm->out_finger, (int) len) != (int) len)
    printf("Seg #9\n");
    return FALSE;

  rstrm->frag_header = (uint32_t *) rstrm->out_base;
  rstrm->out_finger = (char *) rstrm->out_base + BYTES_PER_FLEXXDR_UNIT;
  return TRUE;
}

static bool_t
flexxdrrec_putbytes (FLEXXDR *flexxdrs, const char *addr, u_int len)
{
   RECSTREAM *rstrm = (RECSTREAM *) flexxdrs->f_private;
   u_int current;

   printf("flexxdrrec_putbytes: putting bytes\n");
   printf("flexxdrrec_putbytes: string is [%s]\n", addr);
   while (len > 0)
   {
     current = rstrm->out_boundary - rstrm->out_finger;
     printf("flexxdrrec_putbytes: Current is [%d]\n", current);
     current = (len < current) ? len : current;
     memcpy(rstrm->out_finger, addr, current);
     printf("flexxdrrec_putbytes: current [%d], string is [%s]\n", current, rstrm->out_finger);
     rstrm->out_finger += current;
     addr += current;
     len -= current;
     if (rstrm->out_finger == rstrm->out_boundary && len > 0)
     {
       rstrm->frag_sent = TRUE;
       if (!flush_out(rstrm, FALSE))
         return FALSE;
     }
   }
   return TRUE;
}

static bool_t /* knows nothing about records! only about input buffer */
fill_input_buf (RECSTREAM *rstrm)
{
  char * where;
  size_t i;
  int len;

  where = rstrm->in_base;
  i = (size_t) rstrm->in_boundary % BYTES_PER_FLEXXDR_UNIT;
  //where += i;
  len = rstrm->in_size - 1;
  printf("Boundary is here [%d]\n", len);
  if ((len = (*(rstrm->readit)) (rstrm->flextcp_handle, where, sizeof(uint32_t))) == -1)
    return FALSE;
  rstrm->in_finger = where;
  //where += sizeof(uint32_t);
  rstrm->in_boundary = where + sizeof(uint32_t);
  return TRUE;
}

static bool_t /* know knothign about records only input buffers */
get_input_bytes(RECSTREAM *rstrm, char * addr, int len)
{
  int current;
  printf("fail #3\n");

  while (len > 0)
  {
    printf("fail #2\n");
    current = rstrm->in_boundary - rstrm->in_finger;
    printf("Current is [%d]\n", current);

    if (current == 0)
    {
      if (!fill_input_buf(rstrm))
        return FALSE;
      continue;
    }
    current = (len < current ) ? len : current;
    printf("number: %d\n", htonl(*((int32_t *)rstrm->in_finger)));
    memcpy(addr, rstrm->in_finger, current);
    rstrm->in_finger += current;
    addr += current;
    len -= current;
    len = 0;
  }
  return TRUE;
}

static bool_t /* consumes input bytes know knothing about records */
skip_input_bytes (RECSTREAM *rstrm, long cnt)
{
  int current;
  printf("Skipping input bytes\n");

  while (cnt > 0)
  {
    current = rstrm->in_boundary - rstrm->in_finger;
    if (current == 0)
    {
      if (!fill_input_buf(rstrm))
        return FALSE;
      continue;
    }
    current = (cnt < current ) ? cnt : current;
    rstrm->in_finger += current;
    cnt -= current;
  }
  return TRUE;
}

static bool_t /* next two bytes of input stream are treated as header */
set_input_fragment (RECSTREAM *rstrm)
{
  uint32_t header;

  if (! get_input_bytes (rstrm, (char *)&header, BYTES_PER_FLEXXDR_UNIT))
    return FALSE;
  
  header = ntohl(header);
  rstrm->last_frag = ((header & LAST_FRAG) == 0) ? FALSE : TRUE;
  if (header == 0)
    return FALSE;
  rstrm->fbtbc = header & ~LAST_FRAG;
  return TRUE;
}

bool_t
flexxdrrec_endofrecord(FLEXXDR *flexxdrs, bool_t sendnow)
{
  RECSTREAM *rstrm = (RECSTREAM *) flexxdrs->f_private;
  u_long len;
  printf("Seg #5\n");
  if (sendnow || rstrm->frag_sent 
      || rstrm->out_finger + BYTES_PER_FLEXXDR_UNIT >= rstrm->out_boundary)
  {
    printf("Seg #6\n");
    rstrm->frag_sent = FALSE;
    printf("output status %d\n", ntohl(*((uint32_t *) rstrm->out_finger)));
    return flush_out(rstrm, TRUE);
  }
  printf("Seg #7\n");

  len = ((char *) rstrm->out_finger - (char *) rstrm->frag_header - BYTES_PER_FLEXXDR_UNIT);
  *rstrm->frag_header = htonl((u_long) len | LAST_FRAG);
  rstrm->frag_header = (uint32_t *) rstrm->out_finger;
  rstrm->out_finger += BYTES_PER_FLEXXDR_UNIT;
  return TRUE;
}

bool_t
flexxdrrec_skiprecord(FLEXXDR *flexxdrs)
{
  RECSTREAM *rstrm = (RECSTREAM *) flexxdrs->f_private;
  printf("fbtbc: %ld\n", rstrm->fbtbc);

  while (rstrm->fbtbc > 0 || (!rstrm->last_frag))
  {
    printf("flexxdrrec_skiprecord: whiel working\n");
    if (!skip_input_bytes (rstrm, rstrm->fbtbc))
        return FALSE;
      rstrm->fbtbc = 0;
      if ((!rstrm->last_frag) && (!set_input_fragment (rstrm)))
        return FALSE;
    }
  rstrm->last_frag = FALSE;
  return TRUE;
}

static u_int
fix_buf_size(u_int s)
{
  if (s < 100)
    s = 4000;
  return RNDUP(s);
}
