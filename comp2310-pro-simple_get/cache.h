#ifndef COMP2310_CACHE_H
#define COMP2310_CACHE_H
#include "parse.h"
#include <stdlib.h>
#include "csapp.h"
struct cache_node
{
	ParsedRequest__ *request; //请求
	char *response;
	int size; // buf大小
	int freq; //访问次数
	struct cache_node *next;
};
int update_lfu(char *buf, int *nbytes, ParsedRequest__ *request);
int update_lru(char *buf, int *nbytes, ParsedRequest__ *request);
int cache_lru(rio_t *remoteRio, char *buf, int nbytes, ParsedRequest__ *request);
int cache_lfu(rio_t *remoteRio, char *buf, int nbytes, ParsedRequest__ *request);
#endif
