CC = gcc
CFLAGS = -g -Wall -Werror 


proxy:
	gcc -o proxy -pthread csapp.c threadpool.c parse.c proxy.c cache.c -O0 -g

# all: proxy

# proxy: proxy.c
# 	$(CC) $(CFLAGS) -o proxy_parse.o -c proxy_parse.c
# 	$(CC) $(CFLAGS) -o proxy.o -c proxy.c 	
# 	$(CC) $(CFLAGS) -o threadpool.o -c threadpool.c
# 	$(CC) $(CFLAGS) -o csapp.o -c csapp.c
# 	$(CC) $(CFLAGS) -o proxy proxy.o csapp.o proxy_parse.o threadpool.o 

clean:
	rm -f proxy *.o

# OBJS = proxy.o csapp.o

# all: proxy tiny

# csapp.o: csapp.c
# 	$(CC) $(CFLAGS) -c csapp.c

# proxy.o: proxy.c  
# 	$(CC) $(CFLAGS) -c proxy.c

# cache.o: cache.c cache.h
# 	$(CC) $(CFLAGS) -c cache.c

# proxy: proxy.o cache.o csapp.o
# 	$(CC) $(CFLAGS) proxy.o cache.o csapp.o -o proxy $(LDFLAGS)

# tiny:
# 	(cd tiny; make clean; make)
# 	(cd tiny/cgi-bin; make clean; make)

# clean:
# 	rm -f *~ *.o proxy core 
# 	(cd tiny; make clean)
# 	(cd tiny/cgi-bin; make clean)
