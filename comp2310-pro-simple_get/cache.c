#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csapp.h"
#include "cache.h"
#include <pthread.h>
#define MAX_CACHE_SIZE 1049000
extern struct cache_node* cacheHead; //缓存链表头节点，该节点只用于索引，不表示实际缓存
extern pthread_mutex_t cache_lock;



//比较两个请求是否一致
int diff_request( ParsedRequest__* request1, ParsedRequest__* request2)
{
	printf("\n%s***PATH*\n%s\n",request1->path,request2->path);
	printf("\n%s***METHOD***\n%s\n",request1->method,request2->method);
	if(strcmp(request1->method,request2->method))
	{
		return 0;
	}
/*	if(strcmp(request1->protocol,request2->protocol))
	{
		return 0;
	}*/

	if(strcmp(request1->host,request2->host))
	{
		return 0;
	}
	//printf("cmp %p, %p\n", request1->path, request2->path);
	if(strcmp(request1->path,request2->path))
	{
		return 0;
	}
	return 1;
}
//输出缓存链表
int print_list_host()
{
	int count=0;
	struct cache_node* node = cacheHead;
	while(node->next)
	{
		node = node->next;
		printf("host%d:%s\n",count,node->request->host);
		count+=1;
	}
}

//读取缓存链表，如果缓存中存在request，则将response复制到buf参数中，并将该节点移到链表尾部
int update_lru(char* buf,int *nbytes, ParsedRequest__* request)
{
	pthread_mutex_lock(&cache_lock);
	struct cache_node* node = cacheHead;	//当前节点，头节点不储存缓存
	struct cache_node* preNode = cacheHead;	//前一个节点
	struct cache_node* curNode = cacheHead;	//找到对应节点暂存
	int f=0,first = 0;


								
	while(node->next)	//遍历链表
	{
		preNode = node;
		node = node->next;

		if(diff_request(node->request,request))	//比较请求和当前缓存节点
		{
			//相同请求
			
			memcpy(buf,node->response,node->size); //拷贝buf
			*nbytes = node->size;
			
			if(!node->next)	//如果该节点已经是尾节点，直接返回
			{
				pthread_mutex_unlock(&cache_lock);
				return 1;
			}
			curNode = node;	//暂存当前节点
			preNode->next = node->next;	//删除当前节点
			f=1;	//f=1表示在链表中找到了节点
		}
	}
	if(f==1)
	{
		node->next = curNode;	//把找到的节点添加到尾部
		curNode->next = NULL;
		curNode->request = request;
		pthread_mutex_unlock(&cache_lock);
		return 1;
	}
	pthread_mutex_unlock(&cache_lock);
	return 0;
}

//将新的请求添加到缓存链表尾部
int cache_lru(rio_t* remoteRio,char* buf,int nbytes,  ParsedRequest__* request)
{
	struct cache_node* node = cacheHead;    //同lfu
	int cacheSum = 0;
	int newSize = nbytes;
	struct cache_node* newNode = (struct cache_node*)malloc(sizeof(struct cache_node));
	newNode->size = newSize;
	
	//TODO: correct this
	newNode->request = request;
	newNode->response = (char*)malloc(newSize);
	memcpy(newNode->response,buf,newSize);


	if(strcmp(request->method,"GET")){
		printf("Not a GET method%ld\n",strlen(newNode->request->protocol));
	}
	printf("llll\n");
	newNode->next = NULL;
	int i=0;
	while(node->next)//遍历链表，计算当前缓存大小
	{
		node = node->next;
		cacheSum += node->size;
		i+=1;	
	}
	printf("cacheSum:%d\n",cacheSum);;
	if(cacheSum+newSize>MAX_CACHE_SIZE) //如果新节点加上缓存大小超过总缓存容量
	{
		while(cacheSum+newSize>MAX_CACHE_SIZE) 	//删除第一个缓存直到缓存容量足够
		{
			cacheSum -= cacheHead->next->size;
			cacheHead->next = cacheHead->next->next;
		}
		node->next = newNode;	//添加新节点到尾部
		return 1;
	}
	else
	{
		node->next = newNode;
		return 1;
	}
	newNode->next = NULL;
	return 0;	
}

//读取缓存链表，如果缓存中存在request，则将response复制到buf参数中，并将该节点移到链表尾部
int update_lfu(char* buf,int *nbytes, ParsedRequest__* request)
{
	printf("HHHH\n");
	pthread_mutex_lock(&cache_lock);
	printf("acquired lock\n");
	struct cache_node* node = cacheHead;	//当前节点，头节点不储存缓存
	struct cache_node* preNode = cacheHead;
		printf("before iter list: %p\n", node->next);
	while(node->next)
	{
		preNode = node;
		node = node->next;
		printf("before diff req\n");
		if (node->request->path) {
		printf("node->request->path: %s\n", node->request->path);

		}
		if(diff_request(node->request,request))
		{
			printf("diff match\n");
			*nbytes = node->size;
			memcpy(buf,node->response,node->size); //拷贝buf
			node->freq+=1;		//该节点被访问的次数加一
			if(!node->next)		//如果已经是尾节点，直接返回
			{
				pthread_mutex_unlock(&cache_lock);
				return 1;
			}
			while(node->freq>=node->next->freq)		//如果该节点访问的次数大于或者等于后一个节点，就交换位置
			{
				preNode->next = node->next;		
				preNode = preNode->next;		
				node->next = node->next->next;	
				if(!node->next)
				{
					pthread_mutex_unlock(&cache_lock);
					return 1;
				}
				preNode->next = node;
			}
			pthread_mutex_unlock(&cache_lock);
			return 1;
		}
	}
	pthread_mutex_unlock(&cache_lock);
	printf("end update lfu\n");
	return 0;
}

//将新的请求添加到缓存链表中
int cache_lfu(rio_t* remoteRio,char* buf,int nbytes, ParsedRequest__* request)
{
	struct cache_node* node = cacheHead;
	int cacheSum = 0;
	int newSize = nbytes;
	struct cache_node* newNode = (struct cache_node*)malloc(sizeof(struct cache_node));
	newNode->freq = 1;
	newNode->size = newSize;
	newNode->request = request;
	newNode->response = (char*)malloc(newSize);
	memcpy(newNode->response,buf,newSize);

	if(strcmp(request->method,"GET")){
		printf("Not a GET method%ld\n",strlen(newNode->request->protocol));
	}
	newNode->next = NULL;
	int i=0;
	while(node->next)
	{
		node = node->next;
		cacheSum += node->size;
		i+=1;
	}
	if(cacheSum+newSize>MAX_CACHE_SIZE)
	{
		while(cacheSum+newSize>MAX_CACHE_SIZE)
		{
			cacheSum -= cacheHead->next->size;
			cacheHead->next = cacheHead->next->next;
		}

	}
	//以上同cache_lru
	node = cacheHead;
	struct cache_node* preNode = cacheHead;
	if(!node->next)  	//链表为空
	{
		cacheHead->next = newNode;
		return 1;
	}
	while(node->next)
	{
		preNode = node;
		node = node->next;
		if(!node)		//如果已经到尾节点，添加到尾部
		{
			preNode->next = newNode;
			return 1;
		}
		if(node->freq>=1)	//找到第一个访问次数大于1的，将头节点添加到它的前面
		{
			
			preNode->next = newNode;
			newNode->next = node;
			return 1;
		}
	}
	return 0;	
}
