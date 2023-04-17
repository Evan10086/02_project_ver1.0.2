#include "udp.h"
 
int main(int argc, const char *argv[])
{
    if (3 != argc){
        printf("Usage: %s <IP> <port>\n", argv[0]);
        return -1;
    }
 
    //创建用户数据报套接字
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sockfd){
        ERRLOG("socket error");
    }
 
    //填充服务器网络信息结构体
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    //网络字节序的端口号 8888  9999  6789
    serveraddr.sin_port = htons(atoi(argv[2]));
    //网络字节序的IP地址，IP地址不能乱填
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    socklen_t serveraddr_len = sizeof(serveraddr);
 
    int nbytes = 0;
    char name[32]={0};
    msg_t msg;
    pid_t pid;
 
    struct sockaddr_in clientaddr;
    memset(&clientaddr,0,sizeof(clientaddr));
    socklen_t clientaddr_len = sizeof(clientaddr);
 
    //输入用户名，完成登陆操作
    printf("请输入登录信息：");
    msg.code = 'd';
    memset(msg.user, 0, M);
    fgets(name, M, stdin);//在终端获取用户名
    strcpy(msg.user,name);
    msg.user[strlen(msg.user) - 1] = '\0'; //清空结尾的 '\n' 
    if (-1 == sendto(sockfd,&msg,sizeof(msg),0, (struct sockaddr *)&serveraddr,serveraddr_len)){  //给服务器发送用户名
        ERRLOG("send error");
    }
 
    //创建进程
    if(-1 == (pid = fork())){
        ERRLOG("fork error");
    }else if(0 == pid){   
        //子进程 接收数据  
        while (1){
            memset(&msg,0,sizeof(msg));
            //接收服务器的应答
            if (-1 == (nbytes=recvfrom(sockfd, &msg, sizeof(msg), 0,(struct sockaddr *)&serveraddr,&serveraddr_len))){
                ERRLOG("recv error");
            }  
            // printf("current ------->%d\n",strcmp(msg.user,name)); 
                       
            if(strcmp(msg.user,name) == -10){               
                continue;
            }else{
                //打印应答信息
                switch(msg.code){
                    case 'd':
                        printf("[%s]登录上线了....\n", msg.user); 
                        break; 
                    case 'q':
                        printf("[%s]:%s\n",msg.user,msg.text);
                        break;
                    case 't':            
                        printf("[%s]退出了....\n", msg.user); 
                    break;
                }  
            } 
        }    






        
        }else if(0 < pid){
            //父进程 发送数据（2、q:群聊操作  3、t:退出操作）
            while(1){
                //在终端获取群聊
                memset(msg.text, 0, N);
                fgets(msg.text, N, stdin);
                msg.text[strlen(msg.text) - 1] = '\0'; //清空结尾的 '\n'               
                if( 0 ==strcmp(msg.text, "quit")){
                    msg.code = 't'; 
                    if (-1 == sendto(sockfd, &msg, sizeof(msg), 0,(struct sockaddr *)&serveraddr,serveraddr_len)){
                        ERRLOG("send error");     
                    }   
                    break;
                }else{
                    msg.code = 'q';    
                }
                //给服务器发送群聊消息
                if (-1 == sendto(sockfd, &msg, sizeof(msg), 0,(struct sockaddr *)&serveraddr,serveraddr_len)){
                        ERRLOG("send error");
                }
            }     
        }      
    //关闭套接字
    close(sockfd);
    return 0;
}