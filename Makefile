all:
	gcc -o chatserver chatserver.c -lpthread
	gcc -o chatclient chatclient.c
	gcc -o chatserveroriginal chatserveroriginal.c

server:
	./chatserver 2006

client:
	./chatclient 2006 localhost

original:
	./chatserveroriginal 2006
