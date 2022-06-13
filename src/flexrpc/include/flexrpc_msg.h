#ifndef _FLEXRPC_MSG_H
#define _FLEXRPC_MSG_H_ 1

#include "flexxdr.h"

#define FLEXRPC_MSG_VERSION                ((u_long) 8)
#define FLEXRPC_SERVICE_PORT        ((u_short) 2048)

enum msg_type {
  CALL = 0,
  REPLY = 1
};

enum reply_stat {
  MSG_ACCEPTED=0,
  MSG_DENIED=1
};

enum accept_stat {
  SUCCESS=0,
  PROG_UNAVAIL=1,
  PROG_MISMATCH=2,
  PROC_UNAVAIL=3,
  GARBAGE_ARGS=4,
  SYSTEM_ERR=5
};

enum reject_stat {
  RPC_MISMATCH=0,
  AUTH_ERROR=1
};

/* Reply of an RPC exchange */
/* Reply to an rpc request that was accepted by the server.
 * Note: there could be an error even though the request was accepted
 * server accepts an RPC requests and sends this message
 */
struct accepted_reply {
  enum accept_stat  ar_stat;
  union {
    struct {
      u_long  low;
      u_long  high;
    } AR_versions;
    struct {
      char *  where;
      flexxdrproc_t proc;
    } AR_results;
  } ru;
#define     ar_results    ru.AR_results
#define     ar_vers       ru.AR_versions
};

/* 
 * Reply to an rpc request that was rejected by the server.
 */
struct rejected_reply {
  enum reject_stat rj_stat;
  union {
    struct {
      u_long low;
      u_long high;
    } RJ_versions;
  } ru;
#define     rj_vers   ru.RJ_versions
#define     rj_why    ru.RJ_why
};


/*
 * Body of a reply to an rpc request
 */
struct reply_body {
  enum reply_stat rp_stat;
  union {
    struct accepted_reply RP_ar;
    struct rejected_reply RP_dr;
  } ru;
#define     rp_acpt   ru.RP_ar
#define     rp_rjct   ru.RP_dr
};

/*
 * Body of an rpc request call.
 */
struct call_body {
  u_long cb_rpcvers;
  u_long cb_prog;
  u_long cb_vers;
  u_long cb_proc;
};

/*
 * The rpc message
 */
struct rpc_msg {
  u_long                          rm_xid;
  enum msg_type                   rm_direction;
  union {
    struct call_body RM_cmb;
    struct reply_body RM_rmb;
  } ru;
#define         rm_call         ru.RM_cmb
#define         rm_reply        ru.RM_rmb
} rpc_msg;

#define         acpted_rply     ru.RM_rmb.ru.RP_ar
#define         rjected_rply    ru.RM_rmb.ru.RP_dr


/*
 * FLEXXDR routing to handle an rpc message.
 * flexxdr_callmsg(FLEXXDR *flexxdrs, struct rpc_msg *cmsg)
 */
extern bool_t   flexxdr_callmsg(FLEXXDR *__flexxdrs, struct rpc_msg *__cmsg) __THROW;

/* 
 * FLEXXDR routine to pre-serialized the static part of a rpc message.
 */
extern bool_t flexxdr_callhdr(FLEXXDR *__flexxdrs, struct rpc_msg *__cmsg) __THROW;

/*
 * FLEXXDR routing to handle a rpc reply
 */
extern bool_t flexxdr_replymsg(FLEXXDR *__flexxdrs, struct rpc_msg *__rmsg) __THROW;

extern bool_t flexxdr_rejected_reply (FLEXXDR *flexxdrs, struct rejected_reply *rr);

extern bool_t flexxdr_accepted_reply (FLEXXDR *flexxdrs, struct accepted_reply *ar);

#endif
