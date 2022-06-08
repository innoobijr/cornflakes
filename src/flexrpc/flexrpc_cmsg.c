#include <string.h>
#include "flexrpc.h"

/* 
 * FLEXXDR call message
 */
bool_t
flexxdr_callmsg(FLEXXDR *flexxdrs, struct rpc_msg *cmsg)
{
  int32_t *buf;
  printf("In call message\n");

  if (flexxdrs->f_op == FLEXXDR_ENCODE)
  {
    buf = FLEXXDR_INLINE(flexxdrs, 4 * BYTES_PER_FLEXXDR_UNIT);

    if (buf != NULL){
      //(void) IFLEXXDR_PUT_LONG (buf, cmsg->rm_xid);
      //(void) IFLEXXDR_PUT_ENUM (buf, cmsg->rm_direction);
      //if (cmsg->rm_direction != CALL)
      //  return FALSE;
      (void) IFLEXXDR_PUT_LONG (buf, cmsg->rm_call.cb_rpcvers);
      if (cmsg->rm_call.cb_rpcvers != FLEXRPC_MSG_VERSION)
        return FALSE;
      (void) IFLEXXDR_PUT_LONG (buf, cmsg->rm_call.cb_prog);
      (void) IFLEXXDR_PUT_LONG (buf, cmsg->rm_call.cb_vers);
      (void) IFLEXXDR_PUT_LONG (buf, cmsg->rm_call.cb_proc);
      return TRUE;
    }
  }

  if (flexxdrs->f_op == FLEXXDR_DECODE)
  {
    printf("Decoding this\n");

    buf = FLEXXDR_INLINE (flexxdrs, 4*BYTES_PER_FLEXXDR_UNIT);
    printf("Init bufferrs\n");
    if (buf != NULL){
      //cmsg->rm_xid = IFLEXXDR_GET_LONG(buf);
      //cmsg->rm_direction = IFLEXXDR_GET_ENUM(buf, enum msg_type);
      //if (cmsg->rm_direction != CALL)
      //{
      //  return FALSE;
      //}
      cmsg->rm_call.cb_rpcvers = IFLEXXDR_GET_LONG(buf);
      printf("the number is: [%ld]\n", cmsg->rm_call.cb_rpcvers);
      if (cmsg->rm_call.cb_rpcvers != FLEXRPC_MSG_VERSION)
      { 
        return FALSE;
      }
      cmsg->rm_call.cb_prog = IFLEXXDR_GET_LONG(buf);
      cmsg->rm_call.cb_vers = IFLEXXDR_GET_LONG(buf);
      cmsg->rm_call.cb_proc = IFLEXXDR_GET_LONG(buf);

      return TRUE;
    }
  }

   if ( 
      flexxdr_u_long (flexxdrs, &(cmsg->rm_xid)) &&
      flexxdr_enum(flexxdrs, (enum_t *) &(cmsg->rm_direction)) &&
      (cmsg->rm_direction == CALL) &&
      flexxdr_u_long (flexxdrs, &(cmsg->rm_call.cb_rpcvers)) &&
      (cmsg->rm_call.cb_rpcvers == FLEXRPC_MSG_VERSION) &&
      flexxdr_u_long (flexxdrs, &(cmsg->rm_call.cb_prog)) && 
      flexxdr_u_long (flexxdrs,&(cmsg->rm_call.cb_vers)) &&
      flexxdr_u_long (flexxdrs, &(cmsg->rm_call.cb_proc)))
     return TRUE;
  return FALSE;
}
