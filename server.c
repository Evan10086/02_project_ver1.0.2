#include "udp.h"
 
//创建节点的函数
int create_node(node_t **phead){
    *phead = (node_t *)malloc(sizeof(node_t));
    if(NULL == *phead){
        printf("内存分配失败\n");
        exit(-1);
    }
    (*phead)->next=NULL;
    return 0;
}
//尾插法
int insert_data_by_tail(node_t *phead,struct sockaddr_in addr){
     if(NULL == phead){
        printf("入参为NULL,请检查\n");
        return -1;
    }
    //将新客户端使用插入链表中
    node_t *pnew = NULL;
    create_node(&pnew);
    pnew->addr = addr;  
    node_t *ptemp =phead;
    while(ptemp->next != NULL){
        ptemp = ptemp->next;
    }
    //让尾结点的指针域指向新节点
    ptemp->next = pnew;
    return 0;
}
 
int main(int argc,const char *argv[]){    
    if(3 != argc){
        printf("Uage:%s <IP><port>\n",argv[0]);
        return -1;
    }
    int sockfd = 0;
    if(-1==(sockfd=socket(AF_INET,SOCK_DGRAM,0))){
        ERRLOG("socket error");
    }
    struct sockaddr_in serveraddr;
    memset(&serveraddr,0,sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(atoi(argv[2]));
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    socklen_t serveraddr_len = sizeof(serveraddr);
 
    if(-1 == bind(sockfd,(struct sockaddr *)&serveraddr,serveraddr_len)){
        ERRLOG("bind error");
    } 
 
    struct sockaddr_in clientaddr,temp_clientaddr;
    memset(&clientaddr,0,sizeof(clientaddr));
    socklen_t clientaddr_len = sizeof(clientaddr);
 
    char name[32] = {0};
    pid_t pid = 0;
    msg_t msg;
    msg_t msg_send;
    //创建头结点
    node_t *phead;
    create_node(&phead);
    phead->addr = clientaddr;
 
    if(-1 == (pid = fork())){
        ERRLOG("fork error");
    }else if(0 == pid){   //子进程 接收数据 （1、d 登录操作 2、q 群聊操作 3、t 退出操作）  
        while(1){
            memset(&msg,0,sizeof(msg));
            if(-1 == recvfrom(sockfd, (void*)&msg, sizeof(msg),0, (struct sockaddr *)&clientaddr,&clientaddr_len)){
                perror("recv error");
            }    
        switch(msg.code){

            // 1、d 登录操作 2、q 群聊操作 3、t 退出操作 
            case 'd':
                printf("[%s]该玩家已上线\n", msg.user);              
                insert_data_by_tail(phead,clientaddr);
                node_t *q=phead->next;                
                while(q != NULL){
                    msg.code='d';
                    if(-1 == sendto(sockfd,&msg,sizeof(msg),0,(struct sockaddr *)&q->addr,sizeof(q->addr))){
                        ERRLOG("send error");
                    }
                    q=q->next;
                }
                break; 
            case 'q':                     
                if(strcmp("管理员",msg.user)!=0){
                printf("[%s]:%s\n",msg.user, msg.text);
                }
                node_t *p = phead->next;                
                while(p != NULL){
                    msg.code='q';
                    if(-1 == sendto(sockfd,(void *)&msg,sizeof(msg),0,(struct sockaddr *)&p->addr,sizeof(p->addr))){
                        ERRLOG("send error");
                    }
                    p=p->next;
                }                
                break; 
            case 't':    
                printf("[%s]:退出了...\n", msg.user);
                node_t *t = phead; 
                node_t *pdel = NULL;               
                while(t->next != NULL){
                    msg.code='t';
                    if( 0 == memcmp(&(t->next->addr), &clientaddr,sizeof(clientaddr))){
                        pdel = t->next;
                         t->next = pdel->next;
                        free(pdel);
                    }else{
                        t = t->next;
                        if(-1 == sendto(sockfd, &msg,sizeof(msg),0,(struct sockaddr *)&t->addr,sizeof(t->addr))){
                            ERRLOG("send error");
                        }
                    }    
                }     
                break;
            }
        }
    }else if(0 < pid){
        //父进程 发送系统消息
        while(1){  
            strcpy(msg_send.user,"管理员");
            memset(msg_send.text,0,N);
            fgets(msg_send.text,N,stdin);
            msg_send.text[strlen(msg_send.text)-1] = '\0';
            msg_send.code = 'q';              
            if(-1 == sendto(sockfd,&msg_send,sizeof(msg_send),0,(struct sockaddr *)&serveraddr,serveraddr_len)){
                ERRLOG("send error");                
            }               
        }
    } 
    kill(pid, SIGKILL);
    wait(NULL);//给子进程回收资源
    exit(0);     
    close(sockfd);
    return 0;
}