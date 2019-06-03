#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <iostream>
#include <sstream>
#include <thread>

extern int errno;
int
main(int argc, char ** argv)
{

  if (argc != 4)
  {
    std::cerr<<"ERROR:MUST BE 4 ARGS";
    return 0;
  }
  /*
  char * temp_input = argv[1];
  char * hostname_or_ip = temp_input;
  temp_input = argv[2];
  int port = atoi(temp_input);
  temp_input = argv[3];
  char * filename = temp_input;
  */
  const char* hostname_or_ip = argv[1];
  int port = atoi(argv[2]);
  const char* filename = argv[3];



  //check if the address is legal//////
  int status;
  struct addrinfo hints;
  struct addrinfo *servinfo;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;//ipv4
  hints.ai_socktype = SOCK_STREAM; //TCP stream sockets
  status = getaddrinfo(argv[1], argv[2], &hints, &servinfo);
  if (status != 0)
    {
      fprintf(stderr, "ERROR: %s\n", gai_strerror(status));
      exit(EXIT_FAILURE);
    }
    ///////////////////

  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);






  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);     // short, network byte order
  serverAddr.sin_addr.s_addr = inet_addr(hostname_or_ip);
  memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));


/*
////timeout//////////////////
fd_set active_fd_set;
struct timeval time_out;
FD_ZERO(&active_fd_set);
FD_SET(sockfd, &active_fd_set);
time_out.tv_sec = 1; //set the timeout to 10s
time_out.tv_usec = 0;
//////////////////////////
*/






  // connect to the server

  if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
    perror("connect");
    return 2;
  }










  struct sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
    perror("getsockname");
    return 3;
  }

  char ipstr[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
  std::cout << "Set up a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << std::endl;


  // send/receive data to/from connection



  //read the file filename
  FILE * file_send = fopen(filename, "rb");
  if(file_send == NULL)
    {
      fprintf(stderr, "ERROR: Can't open the file\n");
      return 4;
    }

///timeout/////////////////////


  int ccc = 1;
  while(ccc > 0)
  {

    ccc = ccc + 1;
    if (ccc > 10)
    {
      std::this_thread::sleep_for(std::chrono::seconds(10));
    }
    


    /*
    ///timeout////////////
    int rs = select(sockfd+1, NULL, &active_fd_set, NULL, &time_out);
    if(rs < 0)
      {
        fprintf(stderr, "ERROR: Select failed\n");
        return 4;
      }
    else if( rs == 0)
      {
        fprintf(stderr, "ERROR: Connection timeout\n");
        return 4;
      }
      ////////////////////
      */


    char buf[1024] = {0};
    memset(buf, '\0', sizeof(buf));
    int break_or_not = fread(buf, sizeof(char), sizeof(buf), file_send);

    if (break_or_not == 0)
    {
      fclose(file_send);
      break;
    }
    send(sockfd, buf, break_or_not, 0);
  }



  close(sockfd);

  return 0;
}
