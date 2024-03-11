#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>

#define N 32
typedef struct
{
    short type; 
    char name[64];
    char data[256];
}MSG;


enum
{
    R = 1, // register
    L, // login
    Q, // query
    H //history
};

void do_register(int sockfd,MSG *msg)
{
    printf("do register...\n");
    msg->type = R;
    printf("name:");
    scanf("%s",msg->name);
    getchar();
    printf("pass:");
    scanf("%s",msg->data);

    if (send(sockfd,msg,sizeof(MSG),0) < 0)
    {
        perror("reg send err\n");
        exit(-1);
    }

    if (recv(sockfd,msg,sizeof(MSG),0) < 0)
    {
        perror("reg recv err\n");
        exit(-1);
    }

    else
    {
        printf("%s\n",msg->data);
    }

    
    
    
}
int do_login(int sockfd,MSG *msg)
{
    printf("do login...\n");
    msg->type = L;
    printf("name:");
    scanf("%s",msg->name);
    getchar();
    printf("pass:");
    scanf("%s",msg->data);
    getchar();

    if (send(sockfd,msg,sizeof(MSG),0) < 0)
    {
        perror("reg send err\n");
        return -1;
    }
    
    if (recv(sockfd,msg,sizeof(MSG),0) < 0)
    {
        perror("reg recv err\n");
        return -1;
    }

    if (strncmp(msg->data,"OK",3) == 0)
    {
        printf("login ok!\n");
        return 1;
    }
    else
    {
        printf("%s\n",msg->data);
    }
    

    return -1;

}

int do_query(int sockfd,MSG *msg)
{
    printf("do query...\n");
    msg->type = Q;
    while (1)
    {
        printf("Input word('#' exit):");
        scanf("%s",msg->data);
        //输入“#”退出查询
        if (strncmp(msg->data,"#",1) == 0)
            break;
        if (send(sockfd,msg,sizeof(MSG),0) < 0)
        {
            perror("send err\n");
            return -1;
        }
        //接受服务器返回的单词注释
        if (recv(sockfd,msg,sizeof(MSG),0) < 0)
        {
            perror("recv err\n");
            return -1;
        }
        printf("%s",msg->data);
    }
    
    
    
    return 1;
}

int do_history(int sockfd,MSG *msg)
{
    printf("do history...\n");
    msg->type = H;
    if (send(sockfd,msg,sizeof(MSG),0) < 0)
    {
        perror("send err\n");
        return -1;
    }
    //接受服务器返回的历史查询记录
    
    //打印历史记录
    while (1)
    {
        recv(sockfd,msg,sizeof(MSG),0);
        if (msg->data[0] == '\0')  
        {     
            break;
        }
        printf("%s\n",msg->data);

    }
    
    return 1;
}
// ip:192.168.137.128 port:10000
int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        printf("example:./client <ip> <port>\n");
        exit(0);
    }
first:
    int sockfd;
    if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        perror("socket err\n");
        exit(-1);
    }

    struct sockaddr_in serveraddr;
    bzero(&serveraddr,0);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    serveraddr.sin_port = htons(atoi(argv[2]));

    if (connect(sockfd,(struct sockaddr *)&serveraddr,sizeof(serveraddr)) < 0)
    {
        perror("connect err\n");
        exit(-1);
    }
    
    int n;
    MSG msg;
    //1级菜单
     
    while (1)
    {
        printf("***************************************************\n");
        printf("*       1.register      2.login      3.quit       *\n");
        printf("***************************************************\n");
        printf("please choose:");
        scanf("%d",&n);
        getchar();

        switch (n)
        {
            case 1:
                do_register(sockfd,&msg);
                break;
            case 2:
                //do_login();
                if (do_login(sockfd,&msg) == 1)
                {
                    goto next;
                }
                
                break;
            case 3:
                close(sockfd);
                exit(0);
                break;
            default:
                printf("Invalid data cmd.\n");
        }
    }

next:
    while (1)
    {
        printf("***************************************************\n");
        printf("*      1.do_query    2.do_history    3.logout     *\n");
        printf("***************************************************\n");
        printf("please choose:");
        scanf("%d",&n);
        getchar();

        switch (n)
        {
            case 1:
                do_query(sockfd,&msg);
                break;
            case 2:
                do_history(sockfd,&msg);
                break;
            case 3:
                close(sockfd);
                goto first;
            default:
                printf("Invalid data cmd.\n");
        }        
    }
    
     

    

    
    return 0;
}
