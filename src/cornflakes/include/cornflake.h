#include "flexrpc_msg.h"

#ifndef _CORNFLAKE_H_
#define _CORNFLAKE_H_

//#define CF_DEBUG(c, f, x...) do { } while (0)
//#define CF_DEBUG0(c, f) do { } while (0)
#define CF_DEBUG(f, x...) fprintf(stderr, "cornflake: "f, ##x);
//#define CF_DEBUG0(f, x...) fprintf(stderr, "conn(%p): " f, x)

// Conneciton handle
typedef struct CFLAKE CFLAKE;

struct CFLAKE {
  struct rpc_msg* msg;
  char * args;
};

typedef void (*cf_dispatch)(CFLAKE *, char *);

struct cf_registry {
  flexrpcprog_t prog;
  flexrpcvers_t vers;
  flexrpcproc_t proc;
  flexxdrproc_t cf_call_schema; /* for SER/DES */
  flexxdrproc_t cf_reply_schema; /* for SER/DES */
  cf_dispatch dispatch;
  struct cf_registry *next;
};

bool_t (*cf_callback)(CFLAKE *);

/*  ---------------------------------------------------------------------
 *           Some struct and discriminated unions for dispatch state
 *  ---------------------------------------------------------------------
 *
 * struct cf_success {};
 * struct cf_failure {};
 *
 * enum dispatch_state {
 *  DISPATCH_SUCCESS = 0;
 *  DISPATCH_FAILED = 1;
 * };
 *
 * union {
 *    struct cf_success success;
 *    struct cf_failure failure;
 * } cf_states;
 *
 * struct discriminator 
 * {
 *    int value;
 *    cf_callback callback;
 * };
 *
 * const struct cf_discriminator dscrm[3] = 
 * {
 *    {(int) DISPATCH_SUCCESS, (cf_callback) success_callback},
 *    {(int) DISPATCH_FAILURE, (cf_callback) failure_callback},
 *    {-1, (cf_callback) 0}
 * };
 *
*/

typedef struct cf_registry cf_registry;


bool_t
cornflake_register(flexrpcprog_t prog, flexrpcvers_t vers, flexrpcproc_t proc, flexxdrproc_t cf_call_schema, flexxdrproc_t cf_reply_schema, cf_dispatch dispatch);

bool_t
cornflake_unregister(flexrpcprog_t prog, flexrpcproc_t proc, flexrpcvers_t vers);

bool_t
cornflake_peek_header(FLEXXDR *flexxdrs, CFLAKE* cf);

flexxdrproc_t
cornflake_get_schema(flexrpcprog_t prog, flexrpcvers_t vers, flexrpcproc_t proc, int flexrpc_direction);
//uint64_t cornflake_alloc_rx(flextcp_pl_flowst *fs); 

//uint64_t cornflake_alloc_rx(flextcp_pl_flowst *fs);

bool_t
cornflake_deserialize(FLEXXDR *flexxdrs, CFLAKE* cf);


bool_t
cornflake_serialize(FLEXXDR *flexxdrs, CFLAKE* cf);

#endif
