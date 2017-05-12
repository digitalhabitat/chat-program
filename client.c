// client.c

// Michael Miller
// CSCI 240
// 12/11/2016

// Try running client and server on the same machine first (using "localhost")
// For different machines make sure you use an available PORT_NUM

// Run server program first
// then run client program and connect to server
// up to 5 clients can connect
// set SHOWSTEPS to 1 for help understanding

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>

#define BUFSIZE 1024
#define SHOWSTEPS 0

int myRecv(int i, int sockfd)
{
  char recv_buf[BUFSIZE];
  int nbyte_recvd;

  if (SHOWSTEPS == 1) printf("\t<sockfd is ON in readfdset>\n");

  memset(recv_buf, 0x0, BUFSIZE);
  if (SHOWSTEPS == 1) printf("\t<recv()>\n");
  if( (nbyte_recvd = recv(sockfd, recv_buf, BUFSIZE, 0)) == 0){
    printf("=== Lost connection with server ===\n\n");
    return(0);
  }
  recv_buf[nbyte_recvd] = '\0';
  printf("%s" , recv_buf);
  fflush(stdout);
  return(1);
}

int mySend(int sockfd ){

  char send_buf[BUFSIZE];

  if (SHOWSTEPS == 1) printf("\t<stdin is ON in readfdset>\n");
  if (SHOWSTEPS == 1) printf("\t<fgets()>\n");
  memset(send_buf, 0x0, BUFSIZE);
  fgets(send_buf, BUFSIZE, stdin);

  if (strcmp(send_buf , "quit\n") == 0) {
    return(0);
  }else {
    send(sockfd, send_buf, BUFSIZE, 0);
    return(1);
  }
}

void myConnect(int *sockfd, struct sockaddr_in *server_addr) {

  // using client example from man-pages getaddrinfo
  // although this way is not the only method to do this
  // it makes for a simple way assign sockadddr_in
  // using string data input
  // plus it can be use to eliminate IPv4-IPv6 dependencies

  char buf[BUFSIZE];
  struct addrinfo *result, *rp;
  struct addrinfo hints;
  int s;

  printf("=== If server is running on same machine use \"localhost\" ===\n\n");
  printf("Enter server host name or IP address: ");
  fgets(buf, BUFSIZE, stdin);
  buf[strlen(buf)-1] = '\0'; //remove newline for getaddrinfo()


  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  // take string arguments and converts it properly
  s = getaddrinfo(buf, "1500", &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(1);
  }

  // use *results from getaddrinfo()
  // iterate through multiple socket settings
  // stop after successful connection
  // code snippet sourced from man-pages getaddrinfo
  for (rp = result; rp != NULL; rp = rp->ai_next) {

    if (SHOWSTEPS == 1) printf("\t<socket()>\n");
    *sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if ((*sockfd) == -1)
    continue;

    if (SHOWSTEPS == 1) printf("\t<connect()>\n");
    if (connect(*sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
    break;                  /* Success */
    close(*sockfd);
  }

  if (rp == NULL) {               /* No address succeeded */
    fprintf(stderr, "Could not connect\n");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(result);

  printf("\n=== Connected to server, enter \"quit\" to end program ===\n\n");
}

int main()
{
  int sockfd, fdmax, i;
  struct sockaddr_in server_addr;
  fd_set allset;
  fd_set readfdset;
  int keepGoing;

  myConnect(&sockfd, &server_addr);

  //clear all bits
  //turn ON bit for stdin in allset
  //turn On bit for sockfd in allset
  //assign fdmax
  FD_ZERO(&allset);
  FD_ZERO(&readfdset);
  FD_SET(0, &allset);
  FD_SET(sockfd, &allset);
  fdmax = sockfd;

  //begin loop
  keepGoing = 1;
  while ( keepGoing ) {

    // select() may alter readfdset
    // reassign readfdset it before select() is called
    readfdset = allset;

    // wait for data to become available
    if (SHOWSTEPS == 1) printf("\t<selct()>\n");
    if(select(fdmax+1, &readfdset, NULL, NULL, NULL) == -1) {
      perror("select");
      exit(1);
    }

    // if user has types something
    if(FD_ISSET(0, &readfdset)) {
      keepGoing = mySend( sockfd );
      continue;
    }

    // if server is has sent something
    // could assume i == 4 but this is safer
    for(i=1; i <= fdmax; i++ ) {
      if(FD_ISSET(i, &readfdset)) {
        keepGoing = myRecv(i, sockfd);
      }
    }
  }
  printf("=== Ending program ===\n");
  close(sockfd);
  return 0;
}
