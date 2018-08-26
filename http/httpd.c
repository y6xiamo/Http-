#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<strings.h>
#include<sys/sendfile.h>
#define MAX 1024
#define HOME_PAGE "home.html"  //首页
#define HOME_ERROR "./wwwRoot/404.html"  //404页面
static void usage(const char* msg)
{
    printf("usage:%s port\n",msg);
}
int startup(int  port)
{

    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0)
    {
        perror("socket");
        exit(2);
    }

    int opt = 1;
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));//允许端口号复用

    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port = htons(port);
    local.sin_addr.s_addr = htonl(INADDR_ANY);//表示本地的任意IP地址

    if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0)
    {
        perror("band");
        exit(3);
    }
    
    if(listen(sock,5) < 0)
    {
        perror("listen");
        exit(4);
    }

    return sock;

}

int get_line(int sock,char line[],int size)
{
    char c = 'a';
    int i = 0;
    ssize_t s = 0;
    while(i < size -1 && c != '\n')
    {
        s = recv(sock,(void*)&c,1,0);
        if(s > 0)
        {
            if(c == '\r')
            {
                //\r->\n or \r\n->\n
                recv(sock,&c,1,MSG_PEEK);//MSG_PEEK窥探模式 
                
                if(c != '\n')
                {
                    c = '\n';//\r
                }
                else
                {
                    recv(sock,&c,1,0);
                }
                
            }//end c == '\r'
            //c == \n
             line[i++] = c;

             
        }//end s>0
        else
        {
            break;
        }
    }
        line[i] = '\0';
        printf("%s", line);
        return i;
}

void clear_header(int sock)
{
    char line[MAX];
   do {
     //   printf("%s",line);
        get_line(sock,line,sizeof(line));
    }while(strcmp(line,"\n") != 0);

}

//响应正确页面 200
void echo_www(int sock,char* path,int size,int* errCode)
{
    clear_header(sock);//处理完请求,直到空行
 
    int fd = open(path,O_RDONLY);
    if(fd < 0)
    {
        *errCode = 404;
        return;
    }
    //发送时加上响应报头
    char line[MAX];
    sprintf(line,"HTTP/1.0 200 OK\r\n");
    send(sock,line,strlen(line),0);//发送响应行

    sprintf(line,"Content_Type:text/html;charset=ISO-8859-1\r\n");
    send(sock,line,strlen(line),0);//发送响应报头

    sprintf(line,"\r\n");
    send(sock,line,strlen(line),0);//发送空行

    if(sendfile(sock,fd,NULL,size) < 0)
    {
        *errCode = 503;
    }//发送响应正文
    close(fd);
}

//service处理错误页面

void bad_request(int sock)
{
   // printf("aaaaaaaaaaaaaaaaaaaaaaaa\n");
    //先判断请求的资源是否存在
    clear_header(sock);
    struct stat st;
    if(stat(HOME_ERROR,&st) < 0)
    {
  //      printf("wwwwwwwwwwwwwwwwwwwww\n");
        return;
    }

    //如果存在，打开文件，并开始发送响应
    int fd = open(HOME_ERROR,O_RDONLY);

   // printf("zzzzzzaxxxxxxxxxx\n");
    char line[MAX];

    //发送时加上响应报头
    sprintf(line,"HTTP/1.0 404 NotFound\r\n");
    send(sock,line,strlen(line),0);//发送响应行

    sprintf(line,"Content_Type:text/html;charset=ISO-8859-1\r\n");
    send(sock,line,strlen(line),0);//发送响应报头

    sprintf(line,"\r\n");
    send(sock,line,strlen(line),0);//发送响应行
    
    sendfile(sock,fd,NULL,st.st_size);
    close(fd);

}

void echo_error(int sock,int code)
{
    switch(code)
    {
    case 404:
        bad_request(sock);
        break;
    case 503:
//       bad_request("./wwwRoot/503.html","HTTP/1.0 503 Service Unavailable\r\n",sock);
        break;
    default:
        break;
    }
}

int exe_cgi(int sock,char path[],char method[],char* query_string)
{
 //   printf("AAAAAAAAAAAAAAAAAAA\n");
    char line[MAX];
    int content_length = -1;

    char method_env[MAX/32];
    char query_string_env[MAX];
    char content_length_env[MAX/16];

    if(strcasecmp(method,"GET") == 0)
    {
        clear_header(sock);
    }
    else
    {
        //POST
        while(strcmp(line,"\n") != 0)
        {
            //找到正文的长度，在报头里
            if(strncmp(line,"Content-Length: ",16) == 0)
            {
                //找到将正文长度保存起来
                content_length = atoi(line+16);      
            }
            get_line(sock,line,sizeof(line));
        }
        if(content_length == -1)
        {
            return 404;;
        }          
    }

    //     printf("method:%s ,path:%s\n",method,path);
    //printf("DDDDDDDDDDDDDD\n");
    //发送时加上请求报头
    sprintf(line,"HTTP/1.0 200 OK\r\n");
    send(sock,line,strlen(line),0);//发送响应行

    sprintf(line,"Content-Type:text/html\r\n");
    send(sock,line,strlen(line),0);//发送响应报头

    sprintf(line,"\r\n");
    send(sock,line,strlen(line),0);//发送空行
    
    //创建两个管道(父进程需要知道子进程的执行结果，但进程是独立的，所以需要进程间通信)
    //input --- 父进程将参数传给子进程
    //output --- 子进程将执行结果返回给父进程
    int input[2];//0表示读端，1表示写端
    int output[2];

    pipe(input);//创建管道
    pipe(output);

    pid_t id = fork();
    if(id < 0)
    {
        return 404;
    }
    else if(id == 0)
    {

   //     printf("EEEEEEEEEEEEE\n");
        //child 从input中读，往output中写
        close(input[1]);
        close(output[0]);

        //将标准输入重定向到input
        //将标准输出重定向到output
        dup2(input[0],0);
        dup2(output[1],1);
       
        
        //method GET[query_string],POST[正文参数]
        //将上面参数通过环境变量传给子进程

        //子进程导出环境变量
         sprintf(method_env,"METHOD=%s",method);
         putenv(method_env);

//         getenv(method_env);
//         printf("method:%s",method_env);
         
         //GET -->query_string  POST -->content_length
         if(strcasecmp(method,"GET") == 0)
         {
             sprintf(query_string_env,"QUERY_STRING=%s",query_string);
             putenv(query_string_env);
         }
         else
         {
             sprintf(content_length_env,"CONTENT_LENGTH=%d",content_length);
             putenv(content_length_env);
         }
    

        //子进程进行程序替换，执行path
    
        // printf("###########################method:%s ,path:%s\n",method,path);
//       printf("程序替换开始\n"); 
        execl(path,path,NULL);
        exit(1);
    }
    else
    {
        //father 往input中写,从output中读
        close(input[0]);
        close(output[1]);

        char buf;
        if(strcasecmp(method,"POST") == 0)
        {
            int i = 0;
            for( ;i < content_length;i++)
            {
                read(sock,&buf,1);
                write(input[1],&buf,1);
            }
        }

        while(read(output[0],&buf,1) > 0)
        {
            send(sock,&buf,1,0);
        }


        waitpid(id,NULL,0);
        close(input[1]);
        close(output[0]);
    }
    return 200;
}

void* handle_request(void *arg)
{
    int sock = (int) arg;
    char line[MAX];
    char method[MAX/32];
    char url[MAX];
    int errCode = 200;
    int cgi = 0;
    char* query_string = NULL;
    char path[MAX];

#ifdef Debug
    do{
        get_line(sock,line,sizeof(line));
        printf("%s",line);
    }while(strcmp(line,"\n") != 0);
#else
    if(get_line(sock,line,sizeof(line)) < 0)
    {
        errCode = 404;
        goto end;
    }
    //get method
    //line[] = Get /index.php?a=10&&b=20 HTTP/1.1
    int line_index = 0;
    int method_index = 0;
    //获取method方法
    while(line_index < sizeof(line) && method_index < sizeof(method)-1 && !isspace(line[line_index]))
    {
        method[method_index] = line[line_index];
        line_index++;
        method_index++;
    }
    method[method_index] = '\0';

   // printf("%s\n",method);
    if(strcasecmp(method,"GET")==0)
    {

    }
    else if(strcasecmp(method,"POST")==0)
    {
        cgi = 1;
    }
    else
    {
        errCode = 404;
        goto end;
    }

    //过滤掉空格
    while(line_index < sizeof(line) && isspace(line[line_index]))
    {
        line_index++;
    }

    //get url
    //line[] = Get /index.php?a=10&&b=20 HTTP/1.1
    int url_index = 0;
    while(url_index < sizeof(url)-1 && line_index < sizeof(line) && !isspace(line[line_index]))
    {
        url[url_index] = line[line_index];
        url_index++;
        line_index++;
    }
   // printf("url:%s\n",url);
    url[url_index] = '\0';

    //处理url,get方法带参也是cgi
    //line[] = Get /index.php?a=10&&b=20 HTTP/1.1
    if(strcasecmp(method,"GET") == 0)
    {
        query_string = url;
        while(*query_string)
        {
            if(*query_string == '?')
            {
                *query_string = '\0';
                query_string++;
                cgi = 1;
                break;
            }
            query_string++;
        }
    }

//    printf("hahha\n");
    //method[GET|POST]  cgi[0|1]  url[]放的是资源的路径  query_string[NULL|arg]放的是参数
    //处理请求资源是否存在
    //url -> wwwroot/a/b/c.html | url -> wootRoot/ 请求根目录返回首页
    sprintf(path,"wwwRoot%s",url);//将wwwRoot拼接到url之前，以命令行形式输出到path
    if(path[strlen(path)-1] == '/')
    {
        strcat(path,HOME_PAGE);//请求的资源是web根目录,自动拼接上首页
    }

   // printf("url:%s\n",url);
    struct stat st;
  //  printf("path:%s\n",path);
    if(stat(path,&st) < 0)//stst接口获取属性
    {
        errCode = 404;
        goto end;
    }
    else
    {
   //     printf("CCCCCCCCCCC\n");
        if(S_ISDIR(st.st_mode))
        {
            //请求的资源是如果是目录
            //每个目录下都有一个缺省的首页
            strcat(path,HOME_PAGE);
        }
        else
        {
            if((st.st_mode & S_IXUSR)||(st.st_mode & S_IXGRP) ||(st.st_mode & S_IXOTH))
            {
                cgi = 1;
            }
        }
        printf("method:%s,path:%s\n",method,path);
        if(cgi == 1)
        {
    //        printf("BBBBBBBBBB\n");
              errCode = exe_cgi(sock,path,method,query_string);//执行cgi方式
        }
        else
        {
            //不是cgi，GET方法，不带参
            echo_www(sock,path,st.st_size,&errCode);
        }

    }

#endif
end:
    if(errCode != 200)
    {
        echo_error(sock,errCode);//返回错误信息
    }
    close(sock);

}
//httpd 8080
int main(int argc,char* argv[])
{
    if(argc != 2)
    {
        usage(argv[0]);
        return 1;
    }
   // printf("success begin\n");
    int listen_sock = startup(atoi(argv[1]));
    //signal(SIGPIPE,SIG_IGN);
    for(;;)
    {
        struct sockaddr_in client;
        int len = sizeof(client);
        int new_sock = accept(listen_sock,(struct sockaddr*)&client,(socklen_t*)&len);
        if(new_sock < 0)
        {
            perror("accept");
            continue;
        }
        pthread_t id;
        pthread_create(&id,NULL,handle_request,(void*)new_sock);
        pthread_detach(id);//线程分离
        
    }
    return 0;
}
