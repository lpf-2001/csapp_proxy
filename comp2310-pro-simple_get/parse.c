#include "csapp.h"
#include "parse.h"

const char *REPLACE_UA = "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/105.0.0.0 Safari/537.36\r\n";
const char *REPLACE_PRXYCONN = "Proxy-Connection: close\r\n";
const char *REPLACE_CONN = "Connection: close\r\n";
//Start of custom Linked List library
//========================================================================================================
list *newlist()
{
    list *vh = (list *)malloc(sizeof(list));
    list *vt = (list *)malloc(sizeof(list));
    ;
    vh->vhead = vt->vhead = vh;
    vh->vtail = vt->vtail = vt;
    vh->next = vt;
    vt->prev = vh;
    vh->prev = vt->next = NULL;
    vh->len = 0;
    return vh;
}

list *put_after(list *pos, void *data)
{
    list *node = (list *)malloc(sizeof(list));
    node->d = data;
    node->next = pos->next;
    node->prev = pos;
    node->vhead = pos->vhead;
    node->vtail = pos->vtail;

    pos->next->prev = node;
    pos->next = node;

    pos->vhead->len++;
    return node;
}

list *put_before(list *pos, void *data)
{
    return put_after(pos->prev, data);
}

list *append(list *pos, void *data)
{
    return put_before(pos->vtail, data);
}

list *at(list *l, int pos)
{
    list *result = l->vhead->next;
    for (int i = 0; i < pos; i++)
        result = result->next;
    return result;
}

list *first(list *p)
{
    return p->vhead->next;
}
list *last(list *p)
{
    return p->vtail->prev;
}

void list_remove(list *pos)
{
    pos->vhead->len--;
    pos->prev->next = pos->next;
    pos->next->prev = pos->prev;
}

int find_first(char *buf, char ch)
{
    int i = 0;
    while (buf[i] != ch)
        i++;
    return i;
}

int find_skip(char *buf, char ch, int skip_times)
{
    int pos = 0;
    for (int i = 0; i < skip_times; i++)
        pos = find_first(buf + pos, ch) + pos + 1;
    pos = find_first(buf + pos, ch) + pos;
    return pos;
}

//End custom Linked List library

//Start of custom string operation library
//=============================================================================================================

char *_strdup(char *buf)
{
    char *b = (char *)malloc(strlen(buf) + 1);
    strcpy(b, buf);
    return b;
}

char *strsection(char *buf, int start, int end)
{
    int copylen = end - start + 1;
    char *b = (char *)malloc(copylen + 1);
    memcpy(b, buf + start, copylen);
    b[copylen] = '\0';
    return b;
}

list *strsplit(char *buf, char delimiter)
{
    int pos = find_first(buf, delimiter);
    list *result = newlist();
    append(result, strsection(buf, 0, pos - 1));
    append(result, strsection(buf, pos + 1, strlen(buf) - 1));
    return result;
}

list *strlines(char *buf)
{
    int start = 0, pos = 0;
    char *line;

    list *result = newlist();

    while (1)
    {
        pos = find_first(buf + start, '\r') + start;
        line = strsection(buf, start, pos - 1);
        append(result, line);
        start = pos + 2;

        if (start >= strlen(buf))
            break;
    }
    return result;
}

char *strtrim(char **pstr)
{
    char *str = *pstr;
    int len = strlen(str);
    for (int i = 0; i < len; i++)
    {
        if (str[i] == ' ' || str[i] == '\t' || str[i] == '\r' || str[i] == '\n')
            *pstr = str + i + 1;
        else
            break;
    }
    for (int i = len - 1; i >= 0; i--)
    {
        if (str[i] == ' ' || str[i] == '\t' || str[i] == '\r' || str[i] == '\n')
            str[i] = '\0';
        else
            break;
    }
    return *pstr;
}
//End custom string operation library


//Create header:value\r\n strings from a list of key:value list
char *CreateHeader(list *l)
{
    int total_length = 0;
    for (list *i = first(l); i != l->vtail; i = i->next)
    {
        list *pair = (list *)i->d;

        // Special case for User-Agent, Proxy-Connection, Connection
        if (strcmp(((char *)first(pair)->d), "User-Agent") == 0)
        {
            total_length += strlen(REPLACE_UA);
            continue;
        }
        // if (strcmp(((char*)first(pair)->d), "Proxy-Connection") == 0)
        // {
        //     total_length += strlen(REPLACE_PRXYCONN);
        //     continue;
        // }
        // if (strcmp(((char*)first(pair)->d), "Connection") == 0)
        // {
        //     total_length += strlen(REPLACE_CONN);
        //     continue;
        // }

        total_length += strlen((char *)first(pair)->d);
        total_length += 1;
        total_length += strlen((char *)last(pair)->d);
        total_length += 2;
    }

    char *result = charr(total_length + 1);
    int pos = 0;
    for (list *i = first(l); i != l->vtail; i = i->next)
    {
        list *pair = (list *)i->d;

        // Special case for User-Agent
        if (strcmp(((char *)first(pair)->d), "User-Agent") == 0)
        {
            memcpy(result + pos, REPLACE_UA, strlen(REPLACE_UA));
            pos += strlen(REPLACE_UA);
            continue;
        }
        // if (strcmp(((char*)first(pair)->d), "Proxy-Connection") == 0)
        // {
        //     memcpy(result + pos, REPLACE_PRXYCONN, strlen(REPLACE_PRXYCONN));
        //     pos += strlen(REPLACE_PRXYCONN);
        //     continue;
        // }
        // if (strcmp(((char*)first(pair)->d), "Connection") == 0)
        // {
        //     memcpy(result + pos, REPLACE_CONN, strlen(REPLACE_CONN));
        //     pos += strlen(REPLACE_CONN);
        //     continue;
        // }

        memcpy(result + pos, first(pair)->d, strlen((char *)first(pair)->d));
        pos += strlen((char *)first(pair)->d);
        result[pos] = ':';
        pos += 1;

        memcpy(result + pos, last(pair)->d, strlen((char *)last(pair)->d));
        pos += strlen((char *)last(pair)->d);
        result[pos] = '\r';
        result[pos + 1] = '\n';
        pos += 2;
    }
    result[pos] = '\0';

    return result;
}

//Create HTTP request line suitable to return to proxy client, from request info
char *CreateReqLine(ParsedRequest__ *req)
{
    char buf[256];
    // sprintf(buf, "%s %s://%s:%s/%s %s\r\n", req->method, req->protocol, req->host, req->port, req->path, req->version);
    sprintf(buf, "%s /%s %s\r\n", req->method, req->path, req->version);

    char *result = (char *)malloc(strlen(buf) + 1);
    memcpy(result, buf, strlen(buf) + 1);
    return result;
}

//Create HTTP request line including all information, from request info
char *CreateReqLine_IncludeHost(ParsedRequest__ *req)
{
    char buf[256];
    sprintf(buf, "%s %s://%s:%s/%s %s\r\n", req->method, req->protocol, req->host, req->port, req->path, req->version);

    char *result = (char *)malloc(strlen(buf) + 1);
    memcpy(result, buf, strlen(buf) + 1);
    return result;
}

//Create full HTTP request message including all information
char *CreateFullRequest_IncludeHost(ParsedRequest__ *req)
{
    char *header_key_values = CreateHeader(req->headers);
    char *header_request_line = CreateReqLine_IncludeHost(req);
    char *result = calloc(strlen(header_key_values) + strlen(header_request_line) + 2 + req->len + 1, 1);
    memcpy(result, header_request_line, strlen(header_request_line));
    memcpy(result + strlen(header_request_line), header_key_values, strlen(header_key_values));
    result[strlen(header_request_line) + strlen(header_key_values)] = '\r';
    result[strlen(header_request_line) + strlen(header_key_values) + 1] = '\n';
    memcpy(result + strlen(header_request_line) + strlen(header_key_values) + 2, req->payload, req->len);
    return result;

    /*
    x\r\n     strlen = 3   [0]
    x\r\n\r\n strlen = 5   [3]
    \r\n                   [8]
    payload                    [10]

    */
}

//Create HTTP request response suitable to return to proxy client
char *CreateFullRequest(ParsedRequest__ *req)
{
    char *header_key_values = CreateHeader(req->headers);
    char *header_request_line = CreateReqLine(req);
    char *result = calloc(strlen(header_key_values) + strlen(header_request_line) + 2 + req->len + 1, 1);
    memcpy(result, header_request_line, strlen(header_request_line));
    memcpy(result + strlen(header_request_line), header_key_values, strlen(header_key_values));
    result[strlen(header_request_line) + strlen(header_key_values)] = '\r';
    result[strlen(header_request_line) + strlen(header_key_values) + 1] = '\n';
    memcpy(result + strlen(header_request_line) + strlen(header_key_values) + 2, req->payload, req->len);
    return result;

    /*
    x\r\n     strlen = 3   [0]
    x\r\n\r\n strlen = 5   [3]
    \r\n                   [8]
    payload                    [10]

    */
}
ParsedRequest__ *ParsedRequest__Parse(char *buf, int *error, int len)
{
    // REQUEST LINE PARSING
    int skip = 0;

    char *method = charr(8);
    char *_url = charr(128);
    char *httpver = charr(8);
    char *protocol = charr(16);
    char *host = charr(32);
    char *port = charr(16);
    char *path = charr(64);
    char *c[1];
    skip = sscanf(buf, "%s %s %s\r\n", method, _url, httpver);
    list *lines = strlines(buf + skip);

    // http://www.some.host.com:port/path/path/path/path
    // URL PARSING
    char *url = _url;
    strcpy(protocol, strsection(url, 0, find_first(url, ':') - 1));
    strcpy(host,
           strsection(url,
                      find_first(url, '/') + 2,
                      find_skip(url, ':', 1) - 1));
    strcpy(port,
           strsection(url,
                      find_skip(url, ':', 1) + 1,
                      find_skip(url, '/', 2) - 1));
    strcpy(path,
           url + find_skip(url, '/', 2) + 1);

    list *headers = newlist();
    for (list *i = lines->vhead->next->next; i != lines->vtail; i = i->next)
    {
        if (!strlen((char *)i->d))
            break;
        list *split = strsplit((char *)i->d, ':');
        strtrim((char **)&(first(split)->d));
        strtrim((char **)&(last(split)->d));
        // printf((char*)(first(split)->d)); printf("\n");
        // printf((char*)(last(split)->d)); printf("\n");
        append(headers, split);
    }

    /*
    GET protocol://host:port/path/file HTTP/1.1
    header:value

    data
    */

    char *pos_data = strstr(buf, "\r\n\r\n");
    pos_data += 4;
    int payload_len = len - (pos_data - buf);

    ParsedRequest__ *req = (ParsedRequest__ *)malloc(sizeof(ParsedRequest__));
    req->method = method;
    req->protocol = protocol;
    req->host = host;
    req->port = port;
    req->path = path;
    req->version = httpver;
    req->headers = headers;
    req->payload = malloc(payload_len);
    memcpy(req->payload, pos_data, payload_len);

#define output(x)        \
    printf((char *)(x)); \
    printf("\n");
    output(req->method);
    output(req->protocol);
    output(req->host);
    output(req->port);
    output(req->path);
    output(req->version);

    // #define ERR_EMPTY(x) if (strcmp((char*)(x), "") == 0) *error = -1;

    //     ERR_EMPTY(req->method);
    //     ERR_EMPTY(req->protocol);
    //     ERR_EMPTY(req->host);
    //     ERR_EMPTY(req->port);
    //     ERR_EMPTY(req->path);
    //     ERR_EMPTY(req->version);

    for (list *x = first(req->headers); x != req->headers->vtail; x = x->next)
    {
        output(first((list *)(x->d))->d);
        output(last((list *)(x->d))->d);
    }

    *error = 0;
    return req;
}
