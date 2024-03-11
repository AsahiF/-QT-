#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <signal.h>
#include <time.h>
#include "sqlite3.h"

#define DATABASE "my.db"
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

void do_client(int sockfd,sqlite3 *db);
void do_register(int sockfd,MSG *msg,sqlite3 *db);
int do_login(int sockfd,MSG *msg,sqlite3 *db);
int do_query(int sockfd,MSG *msg,sqlite3 *db);
int do_history(int sockfd,MSG *msg,sqlite3 *db);

// ip:192.168.137.128 port:10000
int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        printf("example:./client <ip> <port>\n");
        exit(0);
    }
    int sockfd;
    sqlite3 *db;

    if (sqlite3_open(DATABASE,&db) != SQLITE_OK)
    {
        printf("%s\n",sqlite3_errmsg(db));
        exit(-1);
    }
    else
    {
        printf("database open success\n");
    }
    
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

    if (bind(sockfd,(struct sockaddr *)&serveraddr,sizeof(serveraddr)) < 0)
    {
        perror("bind err\n");
        exit(-1);
    }

    int clientfd;
    int n;
    MSG msg;
    int acceptfd;
    pid_t pid;
    socklen_t addrlen = sizeof(serveraddr);
    //设置服务器为监听模式
    if (listen(sockfd,5) < 0)
    {
        perror("listen err\n");
        exit(-1);
    }
    //处理僵尸进程
    signal(SIGCHLD,SIG_IGN);
    while (1)
    {
        if ((acceptfd = accept(sockfd,(struct sockaddr *)&serveraddr,&addrlen)) < 0)
        {
            perror("accept err\n");
            exit(-1);
        }
        printf("client connct success!\n");
        if ((pid = fork()) < 0)
        {
            perror("fork err\n");
            exit(-1);
        }
        
        //子进程
        else if (pid == 0)
        {
            //专门处理客户端消息
            close(sockfd);
            do_client(acceptfd,db);
        }
        //父进程
        else
        {
            //用来接受客户端请求
            close(acceptfd);
        }
        
    }
    


    
    return 0;
}

void do_client(int acceptfd,sqlite3 *db)
{
    MSG msg;
    while (recv(acceptfd,&msg,sizeof(msg),0) > 0)
    {
        printf("name:%s,pass:%s\n",msg.name,msg.data);
        switch (msg.type)
        {
        case R:
            do_register(acceptfd,&msg,db);
            break;
        case L:
            do_login(acceptfd,&msg,db);
            break;
        case Q:
            do_query(acceptfd,&msg,db);
            break;
        case H:
            do_history(acceptfd,&msg,db);
            break;
        default:
            printf("Invalid cmd.\n");
            break;
        }
    }
    printf("client eixt..\n");
    close(acceptfd);
    exit(0);
    

}

void do_register(int sockfd,MSG *msg,sqlite3 *db)
{
    char * errmsg;
    char sql[128];
    
    sprintf(sql,"insert into user values('%s','%s');",msg->name,msg->data);
    printf("%s\n",sql);
    if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
    {
        printf("%s\n",errmsg);
        strcpy(msg->data,"user already exist\n");
    }
    else
    {
        printf("regist OK!\n");
        strcpy(msg->data,"OK!");
    }
    if (send(sockfd,msg,sizeof(MSG),0) < 0)
    {
        perror("reg send err\n");
        exit(-1);
    }
    
    //printf("do register...\n");
}
int do_login(int sockfd,MSG *msg,sqlite3 *db)
{
    char * errmsg;
    char sql[128];
    int nrow;
    int nclo;
    char **resultable;
    sprintf(sql,"select * from user where name ='%s' and pass = '%s';",msg->name,msg->data);
    printf("%s\n",sql);
    if(sqlite3_get_table(db,sql,&resultable,&nrow,&nclo,&errmsg) != SQLITE_OK)
    {
        printf("%s\n",errmsg);
        return -1;
    }
    else//查询语句执行成功
    {
        printf("get_table ok!\n");
    }
    if (nrow == 1)
    {
        printf("login OK!\n");
        strcpy(msg->data,"OK");
    }
    
    if (nrow == 0)
    {
        printf("login failed!\n");
        strcpy(msg->data,"name/pass wrong!");
    }
    
    if (send(sockfd,msg,sizeof(MSG),0) < 0)
    {
        perror("reg send err\n");
        return -1;
    }
    return 1;
}
int do_searchword(int sockfd,MSG *msg,char word[])
{
    // char dict[64] = "dict.txt";
    FILE *fp;
    int len = 0;
    char temp[512] = {};
    int result;
    
    if ((fp = fopen("dict.txt","r")) == NULL)
    {
        perror("file open failed\n");
        strcpy(msg->data,"failed to opeb dict.txt");
        send(sockfd,msg,sizeof(MSG),0);
        return -1;
    }
    len = strlen(word);
    printf("%s , len = %d\n",word,len);
    char *p;
    //读文件，查单词 
    while (fgets(temp,512,fp) != NULL)
    {
        result = strncmp(temp,word,len);
        if (result < 0)
        {
            continue;
        }
        if (result > 0 || ((result == 0) && (temp[len] != ' ')))
        {
            break;
        }
        p = temp + len;
        //找到了单词
        while (*p == ' ')
        {
            p++;
        }
        //跳跃所有空格
        strcpy(msg->data,p);
        fclose(fp);
        return 1;
    }
    fclose(fp);
    return 0;
}

int get_date(char *date)
{
    time_t t;
    struct tm *tp;
    time(&t);
    //进行时间格式转换
    tp = localtime(&t);
    sprintf(date,"%d-%d-%d %d:%d:%d",tp->tm_year + 1900,tp->tm_mon + 1,tp->tm_mday,
            tp->tm_hour,tp->tm_min,tp->tm_sec);
    
    return 0;
}
int do_query(int sockfd,MSG *msg,sqlite3 *db)
{
    char word[128];
    int found = 0;
    char date[64];
    char sql[128];
    char * errmsg;
    //取出需要查询的单词
    strcpy(word,msg->data);
    //单词查询操作
    found = do_searchword(sockfd,msg,word);
    //找到了单词,将用户名，单词，时间存到历史记录中
    if (found == 1)
    {
        //获取系统时间
        get_date(date);

        sprintf(sql,"insert into record values('%s','%s','%s');",msg->name,date,word);
        printf("%s\n",sql);

        if (sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
        {
            printf("%s\n",errmsg);
            return -1;
        }
        
    }
    //没有找到单词
    else
    {
        strcpy(msg->data,"Not found!\n");
    }

    if (send(sockfd,msg,sizeof(MSG),0) < 0)
    {
        perror("send err\n");
        return -1;
    }
    
    
    return 1;
}
//得到查询结果，并将历史记录发送给客户端
int history_callback(void *arg,int f_num,char ** f_value,char ** f_name)
{
    int sockfd;
    MSG msg;
    sockfd = *((int *)arg);
    sprintf(msg.data,"%s : %s , %s",f_value[0],f_value[1],f_value[2]);
    printf("%s\n",msg.data);
    send(sockfd,&msg,sizeof(MSG),0);
    return 0;
    
}
int do_history(int sockfd,MSG *msg,sqlite3 *db)
{
    char name[64];
    char sql[128] = {};
    char * errmsg;
    //取出需要查询历史记录的用户名
    strcpy(name,msg->name);

    sprintf(sql,"select * from record where name = '%s';",name);
    printf("%s\n",sql);

    if (sqlite3_exec(db,sql,history_callback,(void *)&sockfd,&errmsg) != SQLITE_OK)
    {
        printf("%s\n",errmsg);
        return -1;
    }
    else
    {
        printf("Query record done.\n");
    }
        
    msg->data[0] = '\0';
    send(sockfd,msg,sizeof(MSG),0);


    return 1;
}