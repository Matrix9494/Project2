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
#include "packet.cpp"

extern int errno;
int
main(int argc, char ** argv)
{

  if (argc != 4)
  {
    std::cerr<<"ERROR:MUST BE 4 ARGS";
    return 0;
  }

  const char* hostname_or_ip = argv[1];
  int port = atoi(argv[2]);
  const char* filename = argv[3];
  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd == -1)
  {
    perror("socket");
    return -1;
  }
  struct sockaddr_in serverAddr;
  socklen_t addr_len = sizeof(struct sockaddr_in);
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);     // short, network byte order
  serverAddr.sin_addr.s_addr = inet_addr(hostname_or_ip);
  memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));


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
  char buf[1024] = {0};
  while(ccc > 0)
  {

    packet send_packet;

    memset(buf, '\0', sizeof(buf));
    int file_not = fread(buf, sizeof(char), sizeof(buf), file_send);

    if (file_not == 0)
    {
      fclose(file_send);
      break;
    }
    generate_packet(send_packet, MAX_SEQ_NUM,MAX_SEQ_NUM,MAX_PKT_LEN,buf);
    std::cout << send_packet.data<< std::endl;


    sendto(sockfd, &send_packet, sizeof(send_packet), 0,(struct sockaddr *)&serverAddr, addr_len);
  }
  memset(buf, '\0', sizeof(buf));
  recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&serverAddr, &addr_len);
  std::cout <<buf<< std::endl;
  close(sockfd);

  return 0;
}
