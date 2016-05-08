#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#define MSG_SIZE 80
#define MAX_CLIENTS 95


void exitClient(int fd, fd_set *readfds, char fd_array[], int *num_clients){
  int i; 
  close(fd);
  FD_CLR(fd, readfds); 
  for (i = 0; i < (*num_clients) - 1; i++)
    if (fd_array[i] == fd)
      break;          
  
  for (; i < (*num_clients) - 1; i++)
      (fd_array[i]) = (fd_array[i + 1]);
  
  (*num_clients)--;
}


int main(int argc, char *argv[]) {
  int i=0;
  int port;
  int result;
  int num_clients = 0;
  int server_sockfd, client_sockfd;
  struct sockaddr_in server_address;
  int addresslen = sizeof(struct sockaddr_in);
  int fd;
  char fd_array[MAX_CLIENTS];
  fd_set readfds, testfds, clientfds;
  char msg[MSG_SIZE + 1];     
  char kb_msg[MSG_SIZE + 10]; 
 
   

  if(argc!=2 ){
    printf("Invalid number of arguments.\nUsage: chatserver PORT \n");
    exit(-1);
  }
  port = atoi(argv[1]);

  printf("\n*** chatserver starting (enter \"quit\" to stop): \n");
  fflush(stdout);

  /* Create and name a socket for the server */
  server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  server_address.sin_port = htons(port);
  if( (bind(server_sockfd, (struct sockaddr *)&server_address, addresslen)) != 0)
    printf(" Error unable to created socket\n");;

  /* Create a connection queue and initialize a file descriptor set */
  listen(server_sockfd, MAX_CLIENTS);
  FD_ZERO(&readfds);
  FD_SET(server_sockfd, &readfds);
  FD_SET(0, &readfds);  
     

     /*  Now wait for clients and requests */
  while (1) {
    testfds = readfds;
    select(FD_SETSIZE, &testfds, NULL, NULL, NULL); //wait for FDs

      /* If there is activity, find which descriptor it's on using FD_ISSET */
    for (fd = 0; fd < FD_SETSIZE; fd++) {
     if (FD_ISSET(fd, &testfds)) {

        if (fd == server_sockfd) { /* New connection request */
          client_sockfd = accept(server_sockfd, NULL, NULL);
          
        if (num_clients < MAX_CLIENTS) {
          FD_SET(client_sockfd, &readfds);
          fd_array[num_clients]=client_sockfd;
          printf("Client %d joined\n",num_clients);
          fflush(stdout);
          num_clients += 1;
          sprintf(msg,"M>> Connected client id %2d \n",client_sockfd);
          write(client_sockfd,msg,strlen(msg));
        }
        else {
          sprintf(msg, "XSorry, too many clients.  Try again later.\n");
          write(client_sockfd, msg, strlen(msg));
          close(client_sockfd);
        }
      }
      else if (fd == 0)  {  /* Stdin/keyboard message */                 
        fgets(kb_msg, MSG_SIZE + 1, stdin);       
        if (strcmp(kb_msg, "quit\n")==0) {
          sprintf(msg, "XServer is shutting down.\n");
          for (i = 0; i < num_clients ; i++) {
            write(fd_array[i], msg, strlen(msg));
            close(fd_array[i]);
          }
        close(server_sockfd);
        exit(0);
        }
        else {
          sprintf(msg, "M>>server: %s", kb_msg);
          for (i = 0; i < num_clients ; i++)
            write(fd_array[i], msg, strlen(msg));
        }
      }
      else if(fd) { 
        result = read(fd, msg, MSG_SIZE);

        if(result==-1) 
          printf("Error reading from client M%2d\n",fd);
        if(result>0){
          msg[result]='\0';
          sprintf(kb_msg,"M>>%2d: ",fd);
          strcat(kb_msg,msg + 1);                                                  
          for(i=0;i<num_clients;i++){
            if (fd_array[i] != fd)  
              write(fd_array[i],kb_msg,strlen(kb_msg));
          }
                           
          printf("%s\n",kb_msg+1);

          if(msg[0] == 'X'){
            exitClient(fd,&readfds, fd_array,&num_clients);
          }   
        }                                   
      }                  
    }
  }
  }
}


