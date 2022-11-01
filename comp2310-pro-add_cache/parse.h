#ifndef PARSE_H
#define PARSE_H

struct __list
{
    struct __list* prev, *next, *vhead, *vtail;
    int len; //ONLY VHEAD STORES THIS VALUE!!!
    void* d;
};

typedef struct __list list;

typedef struct 
{
    char* method, *protocol, *host, *port, *path, *version, *buf;
    list* headers; 
    char* payload;
    int len;
    //list< list[char*,2] >
}ParsedRequest__;


list* newlist();
list* put_after(list* pos, void* data);
list* put_before(list* pos, void* data);
list* append(list* pos, void* data);
list* at(list* l, int pos);
void list_remove(list* pos);
int find_first(char* buf, char ch);
int find_skip(char* buf, char ch, int skip_times);
char* _strdup(char* buf);
char* strsection(char* buf, int start, int end);
list* strsplit(char* buf, char delimiter);
list* strlines(char* buf);
list* first(list* p);
list* last(list* p);
char* strtrim(char** pstr);


#define charr(n) ((char*) malloc(n))
char* CreateHeader(list* l);
char* CreateReqLine(ParsedRequest__* req);
char* CreateFullRequest(ParsedRequest__* req);
ParsedRequest__* ParsedRequest__Parse(char* buf, int* error, int len);



#endif