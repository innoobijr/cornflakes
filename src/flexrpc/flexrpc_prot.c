#include "flexrpc.h"

/*************** FLEXXDR RPC MESSAGE ******/
/*
 * FLEXXDR the MSG_ACCEPTED psrto fot reply message union
 */
bool_t
flexxdr_accepted_reply (FLEXXDR *flexxdrs, struct accepted_reply *ar)
{
  if (!flexxdr_enum(flexxdrs, (enum_t *) & (ar->ar_stat)))
    return FALSE;
  switch (ar->ar_stat)
  {
    case SUCCESS:
      {
        printf("Parsing on success: address [%p]\n", ar->ar_results.where);

      return (ar->ar_results.proc(flexxdrs, ar->ar_results.where));}
    case PROG_MISMATCH:
      if (!flexxdr_u_long(flexxdrs, &(ar->ar_vers.low)))
        return FALSE;
      return (flexxdr_u_long(flexxdrs, &(ar->ar_vers.high)));
    default:
      return TRUE;
  }
  return TRUE;
}

/*
 * FLEXXDR the MSG_DENIED part of a reply messsage union
 */
bool_t
flexxdr_rejected_reply(FLEXXDR *flexxdrs, struct rejected_reply *rr)
{
  if (!flexxdr_enum (flexxdrs, (enum_t *) & (rr->rj_stat)))
    return FALSE;
  switch (rr->rj_stat)
    {
    case RPC_MISMATCH:
      if (!flexxdr_u_long (flexxdrs, &(rr->rj_vers.low)))
        return FALSE;
      return flexxdr_u_long (flexxdrs, &(rr->rj_vers.high));
    }
  return FALSE;
}

static const struct flexxdr_discrim reply_dscrm[3] =
{
  {(int) MSG_ACCEPTED, (flexxdrproc_t) flexxdr_accepted_reply},
  {(int) MSG_DENIED, (flexxdrproc_t) flexxdr_rejected_reply},
  {__dontcare__, NULL_flexxdrproc_t}};

/*
 * XDR a reply message
 */
bool_t
flexxdr_replymsg (FLEXXDR *flexxdrs, struct rpc_msg *rmsg)
{
  if (rmsg->rm_direction == REPLY){
    printf("the replymsg hasnt failed yet\n");
    bool_t res = flexxdr_union (flexxdrs, (enum_t *) & (rmsg->rm_reply.rp_stat),
                      (caddr_t) & (rmsg->rm_reply.ru), reply_dscrm,
                      NULL_flexxdrproc_t);
    if (res != TRUE){
      printf("Success\n");
    }
    return res;
  }
  return FALSE;
}

/* 
 * serialized the static poart of a call message header.
 * The field include: rm_xid, rm_direction, rpcvers, porg, and vers*/
bool_t
flexxdr_callhdr(FLEXXDR *flexxdrs, struct rpc_msg *cmsg)
{
  cmsg->rm_direction = CALL;
  cmsg->rm_call.cb_rpcvers = FLEXRPC_MSG_VERSION;
  if (
      (flexxdrs->f_op == FLEXXDR_ENCODE) &&
      flexxdr_u_long(flexxdrs, &(cmsg->rm_xid)) &&
      flexxdr_enum (flexxdrs, (enum_t *) & (cmsg->rm_direction)) &&
      flexxdr_u_long (flexxdrs, &(cmsg->rm_call.cb_rpcvers)) &&
      flexxdr_u_long (flexxdrs, &(cmsg->rm_call.cb_prog)))
    return flexxdr_u_long(flexxdrs, &(cmsg->rm_call.cb_vers));return FALSE;
}

/* some client utility not yet defined
 * static void accepted()
 * static void rejected()
 * void _seterr_reply()
 */

