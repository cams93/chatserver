typedef struct Server{
  int num_clients;
  int server_sockfd;
  int client_sockfd;
  char * fd_array;
  fd_set readfds, testfds;
} Server;

void exitClient(int fd, fd_set *readfds, char fd_array[], int *num_clients);
void * newClient(void *server);
void * listenClient(void *server);