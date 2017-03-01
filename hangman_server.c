#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "hangman.h"

#define DEST_PORT 9999

int main(int argc, char* argv[])
{
  int listen_socket;
  int new_socket;
  struct sockaddr_in source_addr;
  struct sockaddr_in dest_addr;
  int error_code;
  socklen_t addr_len;
  int c;

// Specify the address family, dest port, and dest ip address
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(DEST_PORT); // Host to network for endianness
  dest_addr.sin_addr.s_addr = inet_addr("0.0.0.0");

  if (argc != 1){
    while ((c = getopt(argc, argv, "p:")) != -1){
      switch (c){
        case 'p':
          dest_addr.sin_port = htons(atoi(optarg));
          break;
        case '?':
          if (optopt == 'p')
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
          else if (isprint(optopt))
            fprintf (stderr, "Unknown option `-%c'.\n", optopt);
          else
            fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
          exit(1);
        default:
          exit(1);
      }
    }
  }

// Create new listen socket
  listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(listen_socket < 0){
    fprintf(stderr, "socket() failed: %s\n", strerror(errno));
    exit(1);
  }

// Bind the socket to the port and address at which we wish to receive data
  error_code = bind(listen_socket, (const struct sockaddr *) &dest_addr, sizeof(dest_addr));
  if(error_code < 0){
    fprintf(stderr, "bind() failed: %s\n", strerror(errno));
    exit(1);
  }

// Set up the socket as a listening socket
  error_code = listen(listen_socket, 10);
  if(error_code < 0){
    fprintf(stderr, "listen() failed: %s\n", strerror(errno));
    exit(1);
  }

  while(1){
    fprintf(stdout, "Waiting for connections...\n");
    addr_len = sizeof(source_addr);
    new_socket = accept(listen_socket,(struct sockaddr *) &source_addr, &addr_len);

    if(new_socket < 0){
      fprintf(stderr, "accept() failed: %s\n", strerror(errno));
      exit(1);
    }

    play_hangman(new_socket, new_socket);
    close(new_socket);
  }

  return 0;
}
