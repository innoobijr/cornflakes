#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "cornflake.h"
#include "flexxdr.h"

#include "templates/oats.h"

#define MAX 80
#define PORT 8080

#define SA struct sockaddr
void read_reply(FLEXXDR *flexxdr, char * name, unsigned int * result, struct rpc_msg * rply, struct printmessage_reply_args* reply_args);

void func(int sockfd, FLEXXDR *flexxdrs, char * buf)
{
    char buff[MAX];
    int n;
    //for (;;) {
      //bzero(buff, sizeof(buff));
      //printf("Enter the string : ");
      //  n = 0;
      //while ((buff[n++] = getchar()) != '\n');
    n = flexxdrs->f_private - flexxdrs->f_base;
    printf("Sending [%d] packets\n", n);
    write(sockfd, flexxdrs->f_base, n);

    // Flip the FLEXXDR
    bzero(buf, sizeof(buf));
    flexxdrs->f_private = flexxdrs->f_base;
    flexxdrs->f_handy = sizeof(char)*80;
    flexxdrs->f_op = FLEXXDR_DECODE;

    read(sockfd, buf, sizeof(char)*80);
    char *name = malloc(sizeof(char)*36);
    unsigned int result;

    struct rpc_msg reply;
    struct printmessage_reply_args reply_args;

    read_reply(flexxdrs, name, &result, &reply, &reply_args);
    printf("My name is %s\n", name);
    //printf("From Server : %s", buff);
    if ((strncmp(buff, "exit", 4)) == 0) {
      printf("Client Exit...\n");
    }
}

void read_reply(FLEXXDR *flexxdr, char * name, unsigned int * result, struct rpc_msg * rply, struct printmessage_reply_args* reply_args)
{
  reply_args->name = &name;
  reply_args->result = result;

  rply->acpted_rply.ar_results.where = (char *) reply_args;
  rply->acpted_rply.ar_results.proc = cornflake_get_schema(OATSPROGRAM, PRINTMESSAGEVER, PRINTMESSAGE,REPLY);

  CFLAKE cf;
  cf.msg = rply;
  cf.args = (char *) &reply_args;

   bool_t res  = cornflake_deserialize(flexxdr, &cf);
}

void prepare_call(FLEXXDR *flexxdr, char ** name, char **title, long ixd)
{
  struct printmessage_call_args call_args;
  call_args.name = name;
  call_args.title = title;

  /* RPC Message */
  struct rpc_msg msg;
  msg.rm_xid = ixd;
  msg.rm_call.cb_rpcvers = 8L;
  msg.rm_direction = CALL;
  msg.rm_call.cb_prog = OATSPROGRAM;
  msg.rm_call.cb_vers = PRINTMESSAGEVER;
  msg.rm_call.cb_proc = PRINTMESSAGE;

  CFLAKE cf;
  cf.msg = &msg;
  cf.args =  (char *) &call_args;

  bool_t res = cornflake_serialize(flexxdr, &cf);

}

int main()
{
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    // FLEXXDR struct
    FLEXXDR flexxdr;
    char *buf = malloc(sizeof(char)*80);

    flexxdrmem_create(&flexxdr, buf, sizeof(char)*80, FLEXXDR_ENCODE);

    cornflake_register(OATSPROGRAM, PRINTMESSAGEVER, PRINTMESSAGE, (flexxdrproc_t)  printmessage_call, (flexxdrproc_t)  printmessage_reply);

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
   
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);
   
    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");

    char * name = "Innocent";
    char * title = "CEO";

    prepare_call(&flexxdr, &name, &title, 22L);

    // function for chat
    func(sockfd, &flexxdr, buf);
   
    // close the socket
    close(sockfd);
}
