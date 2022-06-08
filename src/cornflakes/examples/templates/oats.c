#include "oats.h"

bool_t
printmessage_call(FLEXXDR* flexxdrs,  char * args){
  int MAX_STRING_SIZE = 255;
  struct printmessage_call_args * arg_ptr = ( struct printmessage_call_args *) args;

  return (flexxdr_string(flexxdrs, arg_ptr->name, MAX_STRING_SIZE) && flexxdr_string(flexxdrs, arg_ptr->title, MAX_STRING_SIZE));

}

bool_t
printmessage_reply(FLEXXDR* flexxdrs, char * args){
  int MAX_STRING_SIZE = 255;
  struct printmessage_reply_args * arg_ptr = ( struct printmessage_reply_args *) args;

  return (flexxdr_string(flexxdrs, arg_ptr->name, MAX_STRING_SIZE) && flexxdr_u_int(flexxdrs, arg_ptr->result));

}
