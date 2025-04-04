#include "../include/utils.h"
#include "../include/server.h"
#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <stdint.h>

//TODO: Declare a global variable to hold the file descriptor for the server socket

//TODO: Declare a global variable to hold the mutex lock for the server socket

//TODO: Declare a gloabl socket address struct to hold the address of the server

/*
################################################
##############Server Functions##################
################################################
*/

/**********************************************
 * init
   - port is the number of the port you want the server to be
     started on
   - initializes the connection acception/handling system
   - if init encounters any errors, it will call exit().
************************************************/
void init(int port) {
   //TODO: create an int to hold the socket file descriptor
   //TODO: create a sockaddr_in struct to hold the address of the server


   /**********************************************
    * IMPORTANT!
    * ALL TODOS FOR THIS FUNCTION MUST BE COMPLETED FOR THE INTERIM SUBMISSION!!!!
    **********************************************/
   
   
   
   // TODO: Create a socket and save the file descriptor to sd (declared above)
   
   // TODO: Change the socket options to be reusable using setsockopt(). 


   // TODO: Bind the socket to the provided port.
  

   // TODO: Mark the socket as a pasive socket. (ie: a socket that will be used to receive connections)

   
   
   
   // We save the file descriptor to a global variable so that we can use it in accept_connection().
   // TODO: Save the file descriptor to the global variable master_fd

   printf("UTILS.O: Server Started on Port %d\n",port);
   fflush(stdout);

}


/**********************************************
 * accept_connection - takes no parameters
   - returns a file descriptor for further request processing.
   - if the return value is negative, the thread calling
     accept_connection must should ignore request.
***********************************************/
int accept_connection(void) {

   //TODO: create a sockaddr_in struct to hold the address of the new connection
  
   
   /**********************************************
    * IMPORTANT!
    * ALL TODOS FOR THIS FUNCTION MUST BE COMPLETED FOR THE INTERIM SUBMISSION!!!!
    **********************************************/
   
   
   
   // TODO: Aquire the mutex lock

   // TODO: Accept a new connection on the passive socket and save the fd to newsock

   // TODO: Release the mutex lock


   // TODO: Return the file descriptor for the new client connection
   
}


/**********************************************
 * send_file_to_client
   - socket is the file descriptor for the socket
   - buffer is the file data you want to send
   - size is the size of the file you want to send
   - returns 0 on success, -1 on failure 
************************************************/
int send_file_to_client(int socket, char * buffer, int size) 
{
    //TODO: create a packet_t to hold the packet data
    packet_t *pack_size = malloc(sizeof(packet_t));
    pack_size->size = size;

    int res;
 

    //TODO: send the file size packet
    res = send(socket, pack_size, sizeof(packet_t), 0);
    if (res == -1) {    // Error check
      return res;
    }


    //TODO: send the file data
    res = send(socket, buffer, size, 0);
    if (res == -1) {    // Error check
      return res;
    }
  
    //TODO: return 0 on success, -1 on failure
    return 0;   // Success

}


/**********************************************
 * get_request_server
   - fd is the file descriptor for the socket
   - filelength is a pointer to a size_t variable that will be set to the length of the file
   - returns a pointer to the file data
************************************************/
char * get_request_server(int fd, size_t *filelength)
{
    //TODO: create a packet_t to hold the packet data
    packet_t *pack_size;
    int res;
    unsigned int size;
 
    //TODO: receive the response packet
    res = recv(fd, pack_size, sizeof(packet_t), 0);
    if (res == -1) {
      perror("recv error");
    }
  
    //TODO: get the size of the image from the packet
    size = pack_size->size;

    //TODO: recieve the file data and save into a buffer variable.
    char *buffer;
    res = recv(fd, buffer, size, 0);
    if (res == -1) {
      perror("recv error");
    }

    //TODO: return the buffer
    return buffer;
}


/*
################################################
##############Client Functions##################
################################################
*/

/**********************************************
 * setup_connection
   - port is the number of the port you want the client to connect to
   - initializes the connection to the server
   - if setup_connection encounters any errors, it will call exit().
************************************************/
int setup_connection(int port)
{
    //TODO: create a sockaddr_in struct to hold the address of the server   

    //TODO: create a socket and save the file descriptor to sockfd
   
    //TODO: assign IP, PORT to the sockaddr_in struct

    //TODO: connect to the server
   
    //TODO: return the file descriptor for the socket
}


/**********************************************
 * send_file_to_server
   - socket is the file descriptor for the socket
   - file is the file pointer to the file you want to send
   - size is the size of the file you want to send
   - returns 0 on success, -1 on failure
************************************************/
int send_file_to_server(int socket, FILE *file, int size) 
{
    //TODO: send the file size packet
    packet_t *pack_size = malloc(sizeof(packet_t));
    pack_size->size = size;
    int res;

    res = send(socket, pack_size, sizeof(packet_t), 0);
    if (res == -1) {    // Error check
      return res;   // Failure
    }

    //TODO: send the file data
    char *buffer;
    res = fread(buffer, size, 1, file);
    if (res == 0) {
      return -1;    // Failure
    }

    res = send(socket, buffer, size, 0);
    if (res == -1) {
      return res;   // Failure
    }

    // TODO: return 0 on success, -1 on failure
    return 0;   // Success  
}

/**********************************************
 * receive_file_from_server
   - socket is the file descriptor for the socket
   - filename is the name of the file you want to save
   - returns 0 on success, -1 on failure
************************************************/
int receive_file_from_server(int socket, const char *filename) 
{
    int res;
    //TODO: create a buffer to hold the file data
    char *buffer;    

    //TODO: open the file for writing binary data
    FILE *fh = fopen(filename, "wb");   
    
    //TODO: create a packet_t to hold the packet data
    packet_t *pack_size;
    
    //TODO: receive the response packet
    res = recv(socket, pack_size, sizeof(packet_t), 0);
    if (res == -1) {
      return res;   // Failure
    }

    //TODO: get the size of the image from the packet
    int size = pack_size->size;  

    //TODO: recieve the file data and write it to the file
    res = recv(socket, buffer, size, 0);
    if (res == -1) {
      return res;   // Failure
    }

    res = fwrite(buffer, size, 1, fh);
    if (res == 0) {
      return -1;    // Failure
    }
    
    //TODO: return 0 on success, -1 on failure
    return 0;   // Success
}

