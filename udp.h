#ifndef __UDP_H__
#define __UDP_H__
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<netinet/in.h>
#include<netinet/ip.h>
#include<arpa/inet.h>
#include<signal.h>
#include<wait.h>
 
#define N 128
#define M 32
 
#define ERRLOG(msg) do{\
    printf("%s %s(%d):", __FILE__, __func__, __LINE__);\
    perror(msg);\
    exit(-1);\
}while(0)
 
typedef struct _Node{
    struct sockaddr_in addr; 
    struct _Node *next;
}node_t;
 
typedef struct _Msg{
    char code;
    char user[M];
    char text[N];
}msg_t;
#endif