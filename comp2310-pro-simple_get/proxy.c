#include <stdio.h>
#include "csapp.h"
#include "threadpool.h"
#include "string.h"
//#include "proxy_parse.h"
#include "parse.h"
#include "cache.h"

#define TRACE(x)  printf("trace:%d\n", x);

FILE* fpp;

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
struct cache_node* cacheHead;       //缓存链表头节点，该节点只用于索引，不表示实际缓存
//int (*update_cache)(char* buf,int *nbytes, struct ParsedRequest* request); 
//int (*add_cache)(rio_t* remoteRio,char* buf,int nbytes, struct ParsedRequest* request);
pthread_mutex_t cache_lock;
//static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/105.0.0.0 Safari/537.36\r\n";

typedef struct {
    rio_t clientrio;
    int connfd;
} ConnectInfo;

int sendErrorMessage(int socket, int status_code)//Proper error handling（引用csapp错误码处理）
{
    char str[1024];
    char currentTime[50];
    time_t now = time(0);

    struct tm data = *gmtime(&now);
    strftime(currentTime, sizeof(currentTime), "%a, %d %b %Y %H:%M:%S %Z", &data);

    switch (status_code)
    {
    case 500: snprintf(str, sizeof(str), "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 115\r\nConnection: keep-alive\r\nContent-Type: text/html\r\nDate: %s\r\nServer: VaibhavN/14785\r\n\r\n<HTML><HEAD><TITLE>500 Internal Server Error</TITLE></HEAD>\n<BODY><H1>500 Internal Server Error</H1>\n</BODY></HTML>", currentTime);
        ////printf("500 Internal Server Error\n");
        Rio_writen(socket, str, strlen(str));
        break;

    default:  return -1;

    }
    return 1;
}


int handleGETrequest(ConnectInfo* info,ParsedRequest__* request,char* buf)
{
    strcpy(buf, CreateFullRequest(request));
    TRACE(4);
    printf("%s\n", buf);

    if (request->port == NULL) {
        ////printf("proxy------------------handleGETrequest remotePort:80\n");
        request->port = Malloc(8 * sizeof(char));
        strcpy(request->port, "80");// Default Remote Server Port
    }

    
    char newbuf[65535]; 
    int nbytes = -1;

    //int cache_find = (*update_cache)(newbuf, &nbytes, request);//查找cache中是否有客户端之前请求的报文

    if(0)//如果cache命中 不加cache一定未命中
    {
        TRACE(5);
        // printf("cache hit: %d\n", nbytes);
        rio_writen(info->connfd, newbuf, nbytes);//将cache中的内容发送给客户端
        // printf("sent\n");
        fprintf(stdout,"\n\n\n Send to client: \n");
        fwrite(newbuf, 1, strlen(buf), stdout);fflush(stdout);
    }
    else
    {
        TRACE(6);
        printf("cache miss\n");
        int remoteSocketID = Open_clientfd(request->host, request->port);//创建与服务器端的连接

        if (remoteSocketID < 0)
            return -1;
        int bytes_send = rio_writen(remoteSocketID, buf, strlen(buf));//否则将客户端的报文转发给服务器端
        fprintf(stdout,"\n\n\n Proxy-send to Server: \n");
        fwrite(buf, 1, strlen(buf), stdout);fflush(stdout);

        rio_t* remoteRio= Malloc(sizeof(rio_t));
        Rio_readinitb(remoteRio, remoteSocketID);//初始化服务器端的Rio
        char buf[104900];
        char *p = buf;
        printf("metdhot: %s\n", request->method); 
        while ((bytes_send = Rio_readnb(remoteRio, p, MAXLINE)) != 0) {
            printf("loop: %d\n", bytes_send);
            p += bytes_send;
        }
        *p = '\0';
        bytes_send = p - buf;

        //bytes_send = Rio_readnb(remoteRio, buf, MAXLINE);//开始读取服务器端的报文，存入buf
        // if(bytes_send>0)         //cache 
        // {
        //     if(bytes_send<=104900)
        //     {
        //         pthread_mutex_lock(&cache_lock);

        //         (*add_cache)(remoteRio,buf,bytes_send,request);//将buf，即服务器端报文存入cache
        //         printf("Cached Buffer: %s\n", buf);
        //         pthread_mutex_unlock(&cache_lock);
        //     }
        // }
        while (bytes_send > 0)
        {
            printf("sent %d bytes\n", bytes_send);
            bytes_send = rio_writen(info->connfd, buf, bytes_send);//将buf，即服务器端的报文转发给客户端
            if (bytes_send < 0)
            {
                perror("Error in sending data to client socket.\n");
                break;
            }

            bzero(buf, MAXLINE);

            bytes_send = Rio_readnb(remoteRio, buf, MAXLINE);//读取服务器端的报文，存入buf
        }
        bzero(buf, MAXLINE);
        close(remoteSocketID);
    }
    return 0;
}

void* proxy_handler(void* args) {

    int n;// Bytes Transferred
    int byte_cnt = 0; /* Counts total bytes received by server */
    ConnectInfo* info = (ConnectInfo*)args;

    Rio_readinitb(&info->clientrio, info->connfd);//初始化Rio
    char* buf = (char*)Calloc(MAXLINE, sizeof(char));

    byte_cnt = Rio_readlineb(&info->clientrio, buf, MAXLINE);//开始逐行读取客户端报文，存入buf
   
    while (byte_cnt > 0)
    {
        n = strlen(buf);
        if (strcmp(buf + n - 4, "\r\n\r\n") != 0)
        {
            ////printf("Carriage Return Not found!\n");
            byte_cnt = Rio_readlineb(&info->clientrio, buf + n, MAXLINE);//逐行读取客户端报文
        }
        else 
        {
            break;
        }
    }

    printf("%s\n", buf);
    byte_cnt = strlen(buf);
    if(byte_cnt > 0) 
    {
        n = byte_cnt;

        int err = 0;
        ParsedRequest__* req = ParsedRequest__Parse(buf,&err, byte_cnt);//创建ParsedRequest
        printf("err: %d\n", err);
        if (err)//解析客户端报文，对req赋值
            sendErrorMessage(info->connfd, 500);// 500 internal error
        else 
        {
            bzero(buf, MAXLINE);
            if (!strncmp(req->method, "GET\0", 4))                                          // GET Request
            {
                TRACE(1);
                if (req->host && req->path && (!strncmp(req->version, "HTTP/1.0", 8)||!strncmp(req->version, "HTTP/1.1", 8)))//若请求格式为GET
                {
                    TRACE(2);
                    byte_cnt = handleGETrequest(info, req,buf);     // 处理GET代理请求
                    if (byte_cnt == -1)
                    {
                        sendErrorMessage(info->connfd, 500);//printf("error\n");
                    }
                }
                else
                {
                    TRACE(3);
                    sendErrorMessage(info->connfd, 500);//printf("error\n");                  // 500 Internal Error
                }
            }

        }
    }
    free(buf);
    return 0;
}

// int init_cache()
// {
//     cacheHead = (struct cache_node*)Malloc(sizeof(struct cache_node));
//     cacheHead->next = NULL;
//     cacheHead->size = 0;
//     cacheHead->request = NULL;
//     cacheHead->response = NULL;
//     cacheHead->freq = 0;
// }

int main(int argc, char** argv)
{
    int listenfd, connfdp;
    fpp = fopen("proxy.txt","w");
    //int threadnum=8;//线程池的线程数量为8
    //init_cache();//初始化cache
    ConnectInfo* info = Malloc(sizeof(ConnectInfo));;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    if (argc != 3 && argc != 2) 
    {
        fprintf(stderr, "Command invalid! Example: ./proxy <port> <replacement_policy>\n");
        exit(0);
    }
    // if (argc == 2 || !strcmp(argv[2], "LRU")) 
    // {
    //     add_cache = cache_lru;
    //     update_cache=update_lru;
    // }
    
    // else if (!strcmp(argv[2], "LFU")) 
    // {
    //     add_cache = cache_lfu;
    //     update_cache=update_lfu;
    // }
    // else
    // {
    //     fprintf(stderr, "Command invalid! <replacement_policy>: LRU or LFU\n");
    //     exit(0);
    // }
    listenfd = Open_listenfd(argv[1]);//对输入的端口建立监听
    //pool_init(threadnum);//初始化线程池
    // pthread_mutex_init(&cache_lock, NULL);
    while (1) 
    {
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = Accept(listenfd, (SA*)&clientaddr, &clientlen); //收到请求后accept
        info->connfd = connfdp;
        //Rio_readinitb(&info->clientrio, connfdp);//初始化Rio
        proxy_handler((void*)info);
        //pool_add_worker(proxy_handler, (void*)info);//调用线程处理请求
    }
    pool_destroy();
    return 0;
}