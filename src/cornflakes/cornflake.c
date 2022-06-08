#include "cornflake.h"

static cf_registry rpc_registry;

bool_t
cornflake_register(flexrpcprog_t prog, flexrpcvers_t vers, flexrpcproc_t proc, flexxdrproc_t cf_call_schema, flexxdrproc_t cf_reply_schema, cf_dispatch dispatch){
  
  CF_DEBUG("registering service (prog=%ld, vers=%ld, proc=%ld)\n", prog, vers, proc);

  cf_registry *reg = &rpc_registry;

  if (reg->vers == 0) {
    CF_DEBUG("registry is empty\n");
    reg->prog = prog;
    reg->vers = vers;
    reg->proc = proc;
    reg->cf_call_schema = cf_call_schema;
    reg->cf_reply_schema = cf_reply_schema;
    reg->dispatch = dispatch;
    return TRUE;
  }

  do {
    reg = reg->next;
  } while (reg != NULL);

  CF_DEBUG("registry is not empty\n");
  reg = malloc(sizeof(cf_registry));
  reg->prog = prog;
  reg->vers = vers;
  reg->proc = proc;
  reg->cf_call_schema = cf_call_schema;
  reg->cf_reply_schema = cf_reply_schema;
  reg->dispatch = dispatch;

  return TRUE;
}


flexxdrproc_t
cornflake_get_schema(flexrpcprog_t prog, flexrpcvers_t vers, flexrpcproc_t proc, int flexrpc_direction){ 

  cf_registry* reg = &rpc_registry;

  if (reg->vers == 0){
    return NULL_flexxdrproc_t;
  }
  while(reg->vers != vers || reg->prog != prog || reg->proc != proc ){
    if (reg->next == NULL) {
      CF_DEBUG("No record found\n");
      return NULL_flexxdrproc_t;
    }
    reg = reg->next;
  }

  if (flexrpc_direction == CALL) {
    return reg->cf_call_schema;
  }

  if (flexrpc_direction == REPLY) {
    return reg->cf_reply_schema;
  }

  return NULL_flexxdrproc_t;
}

bool_t cornflake_serialize(FLEXXDR *flexxdrs, CFLAKE *cf)
{
  struct rpc_msg *cmsg = cf->msg;
  char *args_ptr = cf->args;
  /******************/
  /* FLEXRPC Client */
  /******************/
  if(cmsg->rm_direction == CALL){
    // This is the client
    //1. Serialize the header
  if (!flexxdr_callhdr(flexxdrs, cmsg))
  {
    return FALSE;
  }

  // Get the schema
  flexxdrproc_t schema = cornflake_get_schema(cmsg->rm_call.cb_prog, cmsg->rm_call.cb_vers, cmsg->rm_call.cb_proc, CALL);
  
  //2. Serialize the rest
  if ((!FLEXXDR_PUTLONG (flexxdrs, (long *) &(cmsg->rm_call.cb_proc))) ||
      (!schema(flexxdrs, args_ptr)))
  {
    // handle the failure case
  }

  return TRUE;
  }

  /*****************************/
  /*      FLEXRPC Server       */
  /*****************************/

  if (cmsg->rm_direction == REPLY){
    // This is the server
    cornflake_peek_header(flexxdrs, cf);
    // The NIC sets the schema
    if (!flexxdr_replymsg(flexxdrs, cmsg))
    { 
      return FALSE;}
    return TRUE;
  }

  return FALSE;
  //1. Create the FLEXXDR struct to encode
  //2. use procedure number to get the serialization scehma
  //3. Uuse schema to start serialization
  //4. Allocate memory the size of schema in TX buffer
  //5. so serialization as dma reads to location in network buffer
  //6. there should not be a failure, but if so, we should just drop the packet i think??
}

bool_t
cornflake_peek_header(FLEXXDR *flexxdrs, CFLAKE *cf){
   return flexxdr_u_long (flexxdrs, &(cf->msg->rm_xid)) && flexxdr_enum (flexxdrs, (enum_t *) &(cf->msg->rm_direction));

}

bool_t cornflake_deserialize(FLEXXDR *flexxdrs, CFLAKE* cf){
  struct rpc_msg *cmsg = cf->msg;
  char * args_pt = cf-> args;
  bool_t status = cornflake_peek_header(flexxdrs, cf);
  if (!status) return FALSE;

  if (cmsg->rm_direction == CALL) {
    // This is the server
    if (!flexxdr_callmsg(flexxdrs, cmsg))
    {
    }

    flexxdrproc_t schema = cornflake_get_schema(cmsg->rm_call.cb_prog, cmsg->rm_call.cb_vers, cmsg->rm_call.cb_proc,cmsg->rm_direction);
    if(!schema(flexxdrs, args_pt))
    {
      return FALSE;
    };
  }

  if (cmsg->rm_direction == REPLY) {
    // this is the client
    if (!flexxdr_replymsg (flexxdrs, cmsg)){
      return FALSE;
    }
  }
  return TRUE;
}
