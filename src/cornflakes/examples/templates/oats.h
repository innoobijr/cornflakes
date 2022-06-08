#include "../../../flexrpc/flexxdr.h"

#ifndef _OATS_H_
#define _OATS_H_

#define OATSPROGRAM 0x20000001
#define PRINTMESSAGEVER 1
#define PRINTMESSAGE 1

struct printmessage_call_args {
  char **name;
  char **title;
};

struct printmessage_reply_args {
  char **name;
  unsigned int * result;
};

extern bool_t
printmessage_call(FLEXXDR* flexxdrs,  char * args);

extern bool_t
printmessage_reply(FLEXXDR* flexxdr, char * args);

#endif
