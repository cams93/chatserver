#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include "chatserver.h"

#define MSG_SIZE 80
#define MAX_CLIENTS 95

pthread_mutex_t mutex_state = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]) {
  char kb_msg[MSG_SIZE + 10];
  char msg[MSG_SIZE + 1];
  int i = 0, port;
  pthread_t threads[2];
  Server mServer;
  mServer.num_clients = 0;
  struct sockaddr_in server_address;
  int addresslen = sizeof(struct sockaddr_in);

  mServer.fd_array = (char*) malloc(sizeof(char) * MAX_CLIENTS);

  if(argc!=2 ){
    printf("Invalid number of arguments.\nUsage: chatserver PORT \n");
    exit(-1);
  }
  port = atoi(argv[1]);

  printf("\n*** chatserver starting (enter \"quit\" to stop): \n");
  fflush(stdout);

  /* Create and name a socket for the server */
  mServer.server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  server_address.sin_port = htons(port);

  if( (bind(mServer.server_sockfd, (struct sockaddr *)&server_address, addresslen)) != 0){
    printf(" Error unable to create socket\n");
  }

  /* Create a connection queue and initialize a file descriptor set */
  listen(mServer.server_sockfd, MAX_CLIENTS);
  FD_ZERO(&(mServer.readfds));
  pthread_mutex_lock(&mutex_state);
  FD_SET(mServer.server_sockfd, &(mServer.readfds));
  FD_SET(0, &(mServer.readfds));
  pthread_mutex_unlock(&mutex_state);
  mServer.testfds = mServer.readfds;

  pthread_create(&threads[0], NULL, newClient, (void*) &mServer);
  pthread_create(&threads[1], NULL, listenClient, (void*) &mServer);

  /*  Now wait for clients and requests */
  while (1) {
    fgets(kb_msg, MSG_SIZE + 1, stdin);
    if (strcmp(kb_msg, "quit\n")==0) {
      printf("Stopping server... \n" );
      sprintf(msg, "XServer is shutting down.\n");
      for (i = 0; i < mServer.num_clients ; i++) {
        write(mServer.fd_array[i], msg, strlen(msg));
        close(mServer.fd_array[i]);
      }
    close(mServer.server_sockfd);
    exit(0);
    }
    else {
      sprintf(msg, "M>>server: %s\n", kb_msg);
      for (i = 0; i < mServer.num_clients ; i++)
        write(mServer.fd_array[i], msg, strlen(msg));
    }
  }
}

void exitClient(int fd, fd_set *readfds, char fd_array[], int *num_clients){
  int i;
  close(fd);
  pthread_mutex_lock(&mutex_state);
  FD_CLR(fd, readfds);
  pthread_mutex_unlock(&mutex_state);
  for (i = 0; i < (*num_clients) - 1; i++)
    if (fd_array[i] == fd)
      break;

  for (; i < (*num_clients) - 1; i++)
      (fd_array[i]) = (fd_array[i + 1]);

  (*num_clients)--;
}

void * newClient(void *server){
  char msg[MSG_SIZE + 1];
  Server* mServer = (Server*) server;
  while(1){
    mServer->client_sockfd = accept(mServer->server_sockfd, NULL, NULL);
    if (mServer->num_clients < MAX_CLIENTS) {
      FD_SET(mServer->client_sockfd, &(mServer->readfds));
      mServer->fd_array[mServer->num_clients]= mServer->client_sockfd;
      printf("Client %d has joined\n",mServer->num_clients);
      fflush(stdout);
      mServer->num_clients += 1;
      sprintf(msg,"M>> Connected client id %2d \n",mServer->client_sockfd);
      write(mServer->client_sockfd,msg,strlen(msg));
    }else {
      sprintf(msg, "XSorry, too many clients.  Try again later.\n");
      write(mServer->client_sockfd, msg, strlen(msg));
      close(mServer->client_sockfd);
    }
    mServer->testfds = mServer->readfds;
  }
}

void * listenClient(void* server){
  char kb_msg[MSG_SIZE + 10];
  char msg[MSG_SIZE + 1];
  int result, i, fd;
  Server* mServer = (Server*) server;
  while(1){
    mServer->testfds = mServer->readfds;
    select(FD_SETSIZE, &(mServer->testfds), NULL, NULL, NULL);
    pthread_mutex_lock(&mutex_state);
    for (fd = 4; fd < FD_SETSIZE; fd++) {
      if(FD_ISSET(fd, &(mServer->testfds))){
        result = read(fd, msg, MSG_SIZE);

        if(result == -1){
          printf("Error reading from client M%2d\n",fd);
        }

        if(result > 0){
          msg[result]='\0';
          sprintf(kb_msg,"M>>%2d: ",fd);
          strcat(kb_msg, msg + 1);
          for(i=0; i<mServer->num_clients; i++){
            if (mServer->fd_array[i] != fd)
            write(mServer->fd_array[i], kb_msg, strlen(kb_msg));
          }

          printf("%s",kb_msg+1);

          if(msg[0] == 'X'){
            exitClient(fd,&(mServer->readfds), mServer->fd_array,&(mServer->num_clients));
          }
        }
      }
    }
    pthread_mutex_unlock(&mutex_state);
  }
}
