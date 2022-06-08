#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "cornflake.h"
#include "flexxdr.h"
#include "templates/oats.h"

#define MAX 80
#define PORT 8080
#define SA struct sockaddr

void get_call(FLEXXDR *flexxdr, char *name, char *title, struct rpc_msg* cmsg);
void set_reply(FLEXXDR *flexxdr, char* name, unsigned int * result, struct rpc_msg* cmsg);

// Function designed for chat between client and server.
void func(int connfd, FLEXXDR *flexxdrs, char * buf)
{
    int n;
    // infinite loop for chat
    //bzero(buf, MAX);
    // read the message from client and copy it in buffer
    read(connfd, buf, 80);
    printf("Buffer size is: [%ld]\n", sizeof(buf));

    // print buffer which contains the client contents
    // copy server message in the buffer
    char *name = malloc(sizeof(char)*36);
    char *title = malloc(sizeof(char)*16);
    struct rpc_msg cmsg;
    unsigned int result = 1;

    get_call(flexxdrs, name, title, &cmsg);
    printf("Buffer size is: %ld\n", flexxdrs->f_private - flexxdrs->f_base);

    // Flip it
    bzero(buf, sizeof(buf));
    flexxdrs->f_private = flexxdrs->f_base;
    flexxdrs->f_handy = sizeof(char)*80;
    flexxdrs->f_op = FLEXXDR_ENCODE;

    set_reply(flexxdrs, name, &result, &cmsg);
    write(connfd, flexxdrs->f_base, flexxdrs->f_private - flexxdrs->f_base);
    /*// and send that buffer to client
        write(connfd, buff, sizeof(buff));
   
        // if msg contains "Exit" then server exit and chat ended.
        if (strncmp("exit", buff, 4) == 0) {
            printf("Server Exit...\n");
            break;
        }
    }*/
}

void get_call(FLEXXDR *flexxdr, char *name, char *title, struct rpc_msg* msg)
{
  struct printmessage_call_args call_args;
  call_args.name = &name;
  call_args.title = &title;

  CFLAKE cf;
  cf.msg = msg;
  cf.args = (char *) &call_args;

  bool_t res = cornflake_deserialize(flexxdr, &cf);
  printf("My name is %s and in am the %s\n", name, title);

}

void set_reply(FLEXXDR *flexxdr, char* name, unsigned int * result, struct rpc_msg* cmsg) 
{
  struct printmessage_reply_args reply_args;
  reply_args.name = &name;
  reply_args.result = result;

  struct rpc_msg rply;
  rply.rm_xid = cmsg->rm_xid;
  rply.rm_direction = REPLY;
  rply.rm_reply.rp_stat = MSG_ACCEPTED;
  rply.acpted_rply.ar_stat = SUCCESS;
  rply.acpted_rply.ar_results.where = (char *) &reply_args;
  rply.acpted_rply.ar_results.proc = cornflake_get_schema(cmsg->rm_call.cb_prog, cmsg->rm_call.cb_vers, cmsg->rm_call.cb_proc, REPLY);

  CFLAKE cf;
  cf.msg = &rply;
  cf.args = (char *) &reply_args;

  bool_t res = cornflake_serialize(flexxdr, &cf);
}
   
// Driver function
int main()
{
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    FLEXXDR flexxdr;
    char *buf = malloc(sizeof(char)*80);

    flexxdrmem_create(&flexxdr, buf, sizeof(char)*80, FLEXXDR_DECODE);
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
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
   
    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");
   
    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);
   
    // Accept the data packet from client and verification
    connfd = accept(sockfd, (SA*)&cli, &len);
    if (connfd < 0) {
        printf("server accept failed...\n");
        exit(0);
    }
    else
        printf("server accept the client...\n");
   
    // Function for chatting between client and server
    func(connfd, &flexxdr, buf);
   
    // After chatting close the socket
    close(sockfd);
}
