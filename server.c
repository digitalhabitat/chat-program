// server.c

// Michael Miller
// CSCI 240
// 12/11/2016

// Try running client and server on the same machine first (using "localhost")
// For different machines make sure you use an available PORT_NUM

// Run server program first
// then run client program and connect to server
// up to 5 clients can connect
// set SHOWSTEPS to 1 for help

#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define PORT_NUM 1500
#define BUFSIZE 1024
#define SETSIZE 5
#define SHOWSTEPS 0

void socketbindlisten(int *listenfd, struct sockaddr_in *server_addr){

  int one = 1;

  // Create TCP socket
  // use the socket() function to create a socket and return a file descriptor
  if (SHOWSTEPS == 1) printf("\t<socket()>\n");
  if ((*listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    perror("socket() error");
    exit(1);
  }
  
  // assign socket address structure for server_addr with well-known port number
  server_addr->sin_family = AF_INET;
  server_addr->sin_addr.s_addr = htons(INADDR_ANY); // wildcard
  server_addr->sin_port = htons(PORT_NUM);

  // prevent socket/port already in use error
  one = 1;
  if (setsockopt(*listenfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
    perror("setsockopt() error");
    exit(1);
  }

  // use bind() function to assign socket adress with socket file descriptor
  // this is to specificy socket data on the local side
  if (SHOWSTEPS == 1) printf("\t<bind()>\n");
  if ((bind(*listenfd, (struct sockaddr *) server_addr, sizeof(*server_addr))) == -1){
    perror("bind() error");
    exit(1);
  }

  // use listen() function to wait for incoming client calls
  if (SHOWSTEPS == 1) printf("\t<listen()>\n");
  if ((listen(*listenfd, 5)) == -1) {
    perror("listen() error");
    exit(1);
  }

  // end Create TCP socket
}

int sendAll(int client[], int maxi, fd_set *allset) {

  char server_buf[BUFSIZE];
  char input_buf[BUFSIZE];
  int j;

  if (SHOWSTEPS == 1) printf("\t<stdin is ON in readfdset>\n");
  
  memset(server_buf, 0x0, BUFSIZE);
  if (SHOWSTEPS == 1)printf("\t<fgets()>\n");

  strncpy(server_buf, "server: ", BUFSIZE);
  fgets(input_buf, BUFSIZE, stdin);


  if (strcmp(input_buf , "quit\n") == 0) {
    return(0);
  }

  // formating message to show source name
  strncat(server_buf, input_buf, BUFSIZE-strlen(input_buf)-1 );
  if (SHOWSTEPS == 1)printf("%s", server_buf);

  // iterate through client[] to relay message(recv_buf)
  for(j = 0; j <= maxi; j++){
    // if client[j] is ON in allset
    if (FD_ISSET(client[j], allset)){

      if (SHOWSTEPS == 1)printf("\t<client[%d] = %d is ON in allset>\n", j, client[j]);    
      if (SHOWSTEPS == 1)printf("\t<send()>\n");

      if (send(client[j], server_buf, strlen(server_buf), 0) == -1) {
         perror("send() error");
      }
    }
  } // end relay message
  return(1);
} // end sendALL()

void recvsend(int client[], int i, int maxi, int sockfd, fd_set *allset  ) {

  char recv_buf[BUFSIZE];
  char array[BUFSIZE];
  ssize_t nbytes;
  int j;

  if (SHOWSTEPS == 1)printf("\t<sockfd \"client[%d]\" = %d is ON in readfdset>\n", i, client[i]);
  memset(recv_buf, 0x0, BUFSIZE);

  if (SHOWSTEPS == 1)printf("\t<recv()>\n");

  //memset(array, 0x0, BUFSIZE);
  snprintf(array, BUFSIZE, "client[%d]: ", i);

  if ( (nbytes = recv(sockfd, recv_buf, BUFSIZE, 0)) == 0) {
    perror("recv() error");
    printf("=== Lost connection with client[%d] ===\n\n", i);

    //if client_addir for a specifided client was saved this could be done
    //but client_addr from acceptsend() only contains the most recently connected
    //printf("%s", inet_ntop(sockfd, ) );
    //printf(":%d\n", ntohs(client_addr.sin_port) );
    close(sockfd);

    // turn OFF the bit for sockfd in allset
    FD_CLR(sockfd, allset);

    // make client[i] available 
    client[i] = -1;
  } else {

    strncat(array, recv_buf, BUFSIZE);
    printf("%s",array);

    // iterate through client[] to relay message(recv_buf)
    for(j = 0; j <= maxi; j++){
      // if client[j] is ON in allset
      if (FD_ISSET(client[j], allset)){
        if (SHOWSTEPS == 1)printf("\t<client[%d] = %d is ON in allset>\n", j, client[j]);
        // if client[j] is not sockfd(messeage source)
        if (client[j] != sockfd) {
          // send recv_buf to client j
          //recv_buf[nbytes] = '\0';
          if (SHOWSTEPS == 1)printf("\t<send() j=%d client[j]=%d>\n",j,client[j]);
          if (send(client[j], array, BUFSIZE, 0) == -1) {
           perror("send() error");
          }
        }
      }
    } // end relay message          
  } // end if recv()
}

int acceptsend(int listenfd, int client[], int *maxi, int *maxfd, fd_set *allset) {

  int connfd;
  struct sockaddr_in client_addr;
  char array[BUFSIZE];
  socklen_t client_len;
  int i;

  if (SHOWSTEPS == 1)printf("\t<listenfd is ON in readfdset>\n");
  if (SHOWSTEPS == 1)printf("\t<accept()>\n");

  // accept new client connection of connfd
  client_len = sizeof(client_addr);
  if ((connfd = accept (listenfd, (struct sockaddr *) &client_addr, &client_len)) == -1){
    perror("accept() error");
    exit(1);
  }
  else {

    // store connfd into client[]
    // iterate through client[] 
    for (i = 0; i < SETSIZE; i++){
      // on first element available for entry (specified by -1)
      if (client[i] < 0) {
        // assign(store) connfd to client[] then break for loop
        client[i] = connfd;
        break;
      }
    }// end store connf into client[]

    // if client[] is full 
    if (i == SETSIZE){

      if (send(connfd, "=== Can not join, server is full ===\n", BUFSIZE, 0) == -1) {
       perror("send() error");
      }
      close(connfd);

      printf("=== Closed incoming connection, server is full ===\n");
      return(1);
    }

    // at accept
    // turn ON the bit for connfd in allset
    FD_SET(connfd, allset);

    printf("=== Established client[%d] connection ", i);
    printf("%s",inet_ntoa(client_addr.sin_addr) );
    printf(":%d ===\n\n",ntohs(client_addr.sin_port) );
    snprintf(array, BUFSIZE, "Sever: Welcome client[%d]!\n", i);

    if (send(client[i], array, BUFSIZE, 0) == -1) {
     perror("send() error");
    }

    // update maxfd for select() to watch if needed
    if (connfd > *maxfd) {
      *maxfd = connfd;
    }
    // update search space for client[] if needed
    if (i > *maxi) {
      *maxi = i;
    }
  } //end accept() new client connection
  return (1);
}

int main(){

  int keepGoing;
  int i, maxi, maxfd, listenfd, sockfd;
  struct sockaddr_in server_addr; 
  fd_set readfdset, allset; 
  int nready, client[SETSIZE];

  //Create TCP sockect
  socketbindlisten(&listenfd, &server_addr);

  // initialize maxfd
  // use maxi for client[] arry index
  // initalize client[]
  // -1 to specify aviablie entry
  // this is where connfd(client sockets) will be stored
  maxfd = listenfd;  
  maxi = -1;  
  for (i = 0; i < SETSIZE; i++) {
    client[i] = -1;
  }

  // clear all bits in allset
  // turn ON the bit for listenfd in allset
  FD_ZERO(&allset);
  FD_SET(listenfd, &allset);
  FD_SET(0, &allset);

  printf("=== Server is now running, enter \"quit\" to end program ===\n\n");

  // use this shell script to output public ip
  printf("=== Fetching public IP === \n\n") ;
  system("./publicip.sh");

  printf("=== Waiting for client to connect to port: ");
  printf("%d ===\n\n", ntohs(server_addr.sin_port));

  // Begin infinite loop
  keepGoing = 1;
  while ( keepGoing ) {
    readfdset = allset;
    
    // use select() to wait until one or more file descriptors
    // become "ready" for specified I/O operations
    // second arg readfdset is watched till a fd is ready on EOF
    // meaning a read() or recv() will not block

    // maxfd is used as an index range for readset
    // readset functions like a binary arrary for
    // valid file descriptors stored in client[]

    if (SHOWSTEPS == 1)printf("\t<select()>\n");
    nready = select(maxfd + 1, &readfdset, NULL, NULL, NULL);
    if (SHOWSTEPS == 1)printf("\t<nready = %d>\n", nready);

    // if bit for listenfd is ON in readfdset
    // this means the server is ready to accept() with out blocking
    if (FD_ISSET(listenfd, &readfdset)) {
      nready = acceptsend(listenfd, client, &maxi, &maxfd, &allset);
      if (--nready <= 0) {
        continue;
      }
    } // end if bit for listenfd is ON in readfdset


    // if bit for fd0(stdin) is ON in readfdset
    // this means something was entered something into stdin(from server)
    if (FD_ISSET(0, &readfdset)) {
      keepGoing = sendAll( client, maxi, &allset);
      if(--nready <= 0){
        continue;
      }
    }

    // check clients for data to read
    for (i = 0; i <= maxi; i++) {
      // if invalid file descriptor number (-1)
      if ( (sockfd = client[i]) < 0){
        continue;
      }
      // if bit for sockfd(client[i]) is ON in readfdset
      // this means a connected client sent something or dissconnected
      if (FD_ISSET(sockfd, &readfdset)) {
        recvsend(client, i, maxi, sockfd, &allset);
        if (--nready <= 0){
          break;
        }
      } // end if bit for sockfd(client[i]) is ON in readfdset
    } // end check client[] for data to read

  } // end while(1)

  if (SHOWSTEPS == 1) printf("\t<close(listenfd)>\n");
  close(listenfd);
  for (i = 0; i <= maxi; i++) {
    if (client[i] >= 0) {
      printf("\t<close(client[%d])>\n",i);
      close(client[i]);
    }
  }
  printf("=== Ending Program ===\n");
  return(0);
}// end main