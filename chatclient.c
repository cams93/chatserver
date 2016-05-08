#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MSG_SIZE 80


int main(int argc, char *argv[]) {
   int i=0;   
   int port;
   struct sockaddr_in server_address;
   int addresslen = sizeof(struct sockaddr_in);
   int fd;
   fd_set readfds, testfds, clientfds;
   char msg[MSG_SIZE + 1];     
   char kb_msg[MSG_SIZE + 10]; 
   
   
   int sockfd;
   int result;
   char hostname[MSG_SIZE];
   struct hostent *hostinfo;
   struct sockaddr_in address;
   char alias[MSG_SIZE];
   int clientid;
   
   
   if(argc!=3 ){
        printf("Invalid number of arguments.\nUsage: chatclient PORT HOSTNAME\n");
        exit(-1);
    }

    port = atoi(argv[1]);
    strcpy(hostname,argv[2]);
    printf("\n*** chatlient starting (enter \"quit\" to stop): \n");
    fflush(stdout);
     
     /* Create a socket*/
     sockfd = socket(AF_INET, SOCK_STREAM, 0);

     /* Name the socket, as agreed with the server */
     hostinfo = gethostbyname(hostname);  /* look for host's name */
     address.sin_addr = *(struct in_addr *)*hostinfo -> h_addr_list;
     address.sin_family = AF_INET;
     address.sin_port = htons(port);

     /* Connect the socket to the server's socket */
     if(connect(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0){
       printf("Error: connecting to server. Make sure that the server is up."); 
       exit(1);
     }
     
     fflush(stdout);
     
     FD_ZERO(&clientfds);
     FD_SET(sockfd,&clientfds);
     FD_SET(0,&clientfds);//stdin
     
     /*  Now wait for messages from the server */
     while (1) {
       testfds=clientfds;
       select(FD_SETSIZE,&testfds,NULL,NULL,NULL);
       
       for(fd=0;fd<FD_SETSIZE;fd++){
          if(FD_ISSET(fd,&testfds)){
             if(fd==sockfd){   //message from the server

                result = read(sockfd, msg, MSG_SIZE);
                msg[result] = '\0';  /* Terminate string with null */
                printf("%s", msg +1);
                
                if (msg[0] == 'X') {                   
                    close(sockfd);
                    exit(0);
                }                             
             }
             else if(fd == 0){  //read keyboard and send the message
                fgets(kb_msg, MSG_SIZE+1, stdin);
                if (strcmp(kb_msg, "quit\n")==0) {
                    sprintf(msg, "XClient is shutting down.\n");
                    write(sockfd, msg, strlen(msg));
                    close(sockfd); //close the current client
                    exit(0); //end program
                }
                else {

                    sprintf(msg, "M%s", kb_msg);
                    write(sockfd, msg, strlen(msg));
                }                                                 
             }          
          }
       }      
     }
}

