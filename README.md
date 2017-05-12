# Terminal Chat C Program

This program is designed to work on GNU/Linux operating systems and provide a simple server/client solution for users to communicate between two terminals. It should demonstrates data structure principles of socket programing and Transmission Control Protocol (TCP) networking.

## Use Case Analysis

As the project title implies this program is designed to work on a GNU/Linux terminal and is intended for users comfortable operating a command-line interface, of beginner skill level programing, and interested in learning about C network programing and Sockets. The expected limitations of this program are multiple clients and rudimentary behavior which would require a higher skill level outside the scope of this project. Below is a command-line procedure to start the program. PORT_NUM may have to be adjusted

#### Compile
```
$ make clean
$ make all
```

#### Server side
```
$ make runserver

== Socket server created ==
== Waiting for client ==
[3:14] connection confirmed! Enter !q to end program
[3:15] Client: Hello!
[3:16] Sever: Hey!
[3:17] Client: Good talking to you gtg bye
[3:17] Server: ok bye
[3:18] == Connection closed ==
```

#### Client side
```
$ make runclient

== Client socket created ==
== connection to server port number: 42 ==
== waiting for confirmation ==
[3:14] connection confirmed! Enter !q to end program
[3:15] Client: Hello!
[3:16] Sever: Hey!
[3:17] Client: Good talking to you gtg bye
[3:17] Server: ok bye
!q
[3:18] == Connection closed ==
```
## Data Design

This program is basically about sockets which is the combination of an IP address and a port number that functions as a file descriptor/handle, an abstract reference to a resource (resource being an endpoint for communication). To elaborate, handles act similar to pointers but are referencing blocks of memory or objects not managed by the program using them.  Below, provides a quick overview of several headers to be include that contain necessary functions and structures to write a basic TCP client and server.

##### Headers

```
<sys/socket.h>
<sys/types.h>
<netinet/in.h>
<arpa/inet.h>
```
##### Functions, Structures, and Types
```
socket(); // similar to fopen()
connect() // use to connect to a listening socket
accept() // use to comfirm connect() request
bind() // use to asociate sockaddr_in data as is own address (being the server)
listen()  // after calling bind() server can now listen for connect() request
select() // use to wait till specified I/O operations are "ready" on a certain file descriptor
recv() // similar to fread()
send() // simlar to fwrite()
close() // close socket file descriptor
getaddrinfo() // use to provide socket() arguments with from string input and eliminate IPv4-IPv6 dependencies
freeaddrinfo() // deallocated results from getaddrinfo()
struct sockaddr_in; // Defined as a “container” for socket address information
htonl() // Important to convert between host and network byte order (little/big-endian).
htons()
ntohl()
ntohs()
inet_ntoa()
```
## Data structure

While the list of functions, structures, data types is quite extensive the main concepts to focus on and explore are IPv4 socket address structure and the socket() function. Like the comment above sockaddr_in{} is defined as  “container” for socket address information.  

From <netinet/in.h> we get struct sockaddr_in
```
struct in_addr {
  in_addr_t   s_addr;           // 32-bit IPv4 address
                                // network byte ordered
};
```
```
struct sockaddr_in {
  uint8_t          sin_len;     // length of structure (16)
  sa_family_t      sin_family;  // AF_NET
  in_port_t        sin_port;    // 16-bit TCP or UDP port number
                                // network byte ordered
  struct in_addr   sin_addr;    // 32-bit IPv4 address
                                // network byte ordered
  char             sin_zero[8]; // unused
};
```
The above code is simplified version so its worth looking into the actual structure header files.
 ```
/* Structure describing an Internet socket address.  */
struct sockaddr_in
  {
    __SOCKADDR_COMMON (sin_);
    in_port_t sin_port;     /* Port number.  */
    struct in_addr sin_addr;    /* Internet address.  */

    /* Pad to size of `struct sockaddr'.  */
    unsigned char sin_zero[sizeof (struct sockaddr) -
         __SOCKADDR_COMMON_SIZE -
         sizeof (in_port_t) -
         sizeof (struct in_addr)];
  };
```
Looking up the macros
```
__SOCKADDR_COMMON (sin_) and __SOCKADDR_COMMON_SIZE in </bits/sockaddr.h>
```
further describes this. Notice how the sin_ usage as and argument for the macro to be transferred as the prefix to family.

```
struct sockaddr_in
{
   sa_family_t sin_family;
   in_port_t sin_port;           /* Port number.  */
   struct in_addr sin_addr;      /* Internet address.  */

   /* Pad to size of `struct sockaddr'.  */
   unsigned char sin_zero[sizeof (struct sockaddr) -
             (sizeof (unsigned short int) -
             sizeof (in_port_t) -
             sizeof (struct in_addr)];
};
```
However daunting it appears, the point was to illustrate how the variable-length socket address structures compared to fixed-length (when length field is included ) socket address structure.
```
int listenSocket;
  listenSocket = socket(AF_INET, SOCK_STREAM, 0);
```
```
  int socket(int domain, int type, int protocol);
```
The domain argument specifies the protocol family (domain) used for communication in other words it provides a identification and location system.

The type argument specifies the communication styles such as Transmission Control Protocol (TCP), User Datagram Protocol (UDP), Internet Protocol (IP), Internet Control Message Protocol (ICMP), RAW, etc.

The protocol argument specifies the protocol to be used with a particular socket type and protocol family. 0 is used to select the system’s default unless many possible protocols exist in which case a particular one must me specified.

## UI Design

The entire program client and server is operated through a command-line interface (CLI). This means that every operation requires text input to perform. This is suitable for a simple program like this one but there were some limitations in user friendliness and and organization. The most obvious issue was how, the space for user input and incoming messages are shared with each other. This means that if a user receives a message while in the middle of typing one, the already typed characters get concatenated with the received message and new line. There is no easy solution to fix this without opting for a different interface all together.

## Algorithm

Beginning with running the server first. The first step is to create a socket file descriptor. This can understood as being similar to creating a new file with fopen(). Before calling bind(), the actual socket data (protocol, address, port) is assigned to a structure type (struct sockaddr_in) specified for a socket file descriptor. Bind is then used to couple the sockaddr_in data type to the file descriptor returned by socket(). Now the listen() function can be called to tell the program to respond to connection request (client calling connect() ) to the specified address and port.

Entering the loop, the select() function is called to wait for a file descriptor to become “ready” to read without out blocking. Blocking is caused by the effect that recv(), send(), or accept() will wait for the single file descriptor weather or not it actually going to happen. Select() is used to signal the program that a list of file descriptor can have I/O operations performed on it--meaning that the file descriptor is actually doing something the server can respond to. The three main things the server is interested in performing I/O operations to are: new client connection, server input, and client input. All of those have a separate file descriptor associated with them. The sever is only interested in reading from them—meaning that there is data we can read.
```
//start server.c
socket()   // create a file descriptor(fd) and specify protocols
           // assign struct sockaddr_in
bind()     //  bind socket fd with sockaddr_in
listen()   // listen for "activity" on socket

while (true) {
  select()
  if (new client connection) {
    accept()
    save client socket file descriptor;
    continue;
  }
  if (server(user) input) {
    fgets()
    send()
    continue;
  }
  if (remote client input) {
    recv()
    send()
    continue;
  }
}end loop
// end server.c
```

The client program has a little less to deal with but has a very similar algorithm. After setting up the socket file descriptor, the client program just needs to connect to the server. Once it has the file descriptor for the server it really only needs to listen for input to read from the user or the sever for incoming data. By using the select() function the program can wait till data is ready to “read” from.

```
//start client.c
socket()      //
connect()     // connects to assumed server address and port

loop {
  select()
  if (client (user) input) {
    fgets()
    send()
    continue;
  }
  if(remote server/client input ) {
    recv()
    continue;
  }
} end loop
//end client.c
```

## General thoughts
During the projecting I felt that the learning curve to building the program with the intended functionality was more difficult than excepted (at least for a first sockets program). The hardest part was understanding which details were relevant. The program initially started out as an echo server then I had to figure out how to handle multiple clients and understand how to avoid blocking I/O operations. This lead to using select() to achieve the specific functionality. Learning how to handle file descriptor sets to use select() was actually harder than setting up the echo server but, it provided more meaningful endeavor. The unexpected key learning points from this project was understanding how things can be treated as a file, I/O multiplexing, and developing a suitable algorithm.
