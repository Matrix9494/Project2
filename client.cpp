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

packet send_packet;
packet recv_packet;
uint16_t cwnd_size = INIT_CWND_SIZE;
uint32_t CURRENT_SEQ_NUM = 1234;
uint32_t CURRENT_ACK_NUM = 0;
bool establishedTCP = 0;
char ALLFILE[100000000];

int main(int argc, char ** argv)
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
  struct sockaddr_in addr;
  socklen_t addr_len = sizeof(struct sockaddr_in);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);     // short, network byte order
  addr.sin_addr.s_addr = inet_addr(hostname_or_ip);
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));


  //first, establish the connection

  char buf[DATA_SIZE] = {0};
  memset(buf, '\0', sizeof(buf));
  CURRENT_ACK_NUM = (recv_packet.head.seq + 1) % (MAX_SEQ_NUM + 1);
  generate_packet(send_packet, CURRENT_SEQ_NUM,CURRENT_ACK_NUM,1,buf);
  if (sendto(sockfd, &send_packet, sizeof(send_packet), 0, (struct sockaddr*)&addr, addr_len) == -1)
  {
    perror("send error");
    return 1;
  }
  cout << "first hand "<<CURRENT_SEQ_NUM<<" "<<CURRENT_ACK_NUM<< endl;

  while (!establishedTCP)
  {
    int bytesRec = recvfrom(sockfd, &recv_packet, MSS, 0, (struct sockaddr*)&addr, &addr_len);
    if ( recv_packet.head.flag == 2 )
    {
      CURRENT_ACK_NUM = recv_packet.head.seq + 1;
      CURRENT_SEQ_NUM = CURRENT_SEQ_NUM + 1;
      establishedTCP = true;
      cout << "finish "<<CURRENT_SEQ_NUM<<" "<<CURRENT_ACK_NUM<< endl;
      break;

    }
  }




  //second, data transmission

  //read the file and save it to ALLFILE
  FILE * file_send = fopen(filename, "rb");
  if(file_send == NULL)
    {
      fprintf(stderr, "ERROR: Can't open the file\n");
      return 4;
    }
  memset(ALLFILE, '\0', sizeof(ALLFILE));
  ini_file = 0;
  while(1)
  {
    memset(buf, '\0', sizeof(buf));
    int file_not = fread(buf, sizeof(char), sizeof(buf), file_send);
    if(file_not == 0)
      break;
    for (int i = 0; i<file_not; i++)
    {
      ALLFILE[i+ini_file] = buf[i];
    }
    ini_file+=file_not;
  }


///timeout/////////////////////

  int jjj = 0;
  while(1)
  {
    jjj++;



    //std::cout << strlen(buf)<< std::endl;
    if (file_not == 0)
    {
      fclose(file_send);
      generate_packet(send_packet, CURRENT_SEQ_NUM,CURRENT_ACK_NUM,4,buf);
      cout<<CURRENT_SEQ_NUM<<" "<<"final"<<endl;
      if(sendto(sockfd, &send_packet, sizeof(send_packet), 0,(struct sockaddr *)&addr, addr_len) == -1)
      {
        perror("send error");
        return 1;
      }
      CURRENT_SEQ_NUM = (CURRENT_SEQ_NUM + sizeof(send_packet.data)) % (MAX_SEQ_NUM + 1);


      break;

    }
    generate_packet(send_packet, CURRENT_SEQ_NUM,CURRENT_ACK_NUM,3,buf);
    cout<<CURRENT_SEQ_NUM<<" "<<"normal"<<endl;
    std::cout << sizeof(send_packet.data)<< std::endl;


    if(sendto(sockfd, &send_packet, sizeof(send_packet), 0,(struct sockaddr *)&addr, addr_len) == -1)
    {
      perror("send error");
      return 1;
    }
    CURRENT_SEQ_NUM = (CURRENT_SEQ_NUM + sizeof(send_packet.data)) % (MAX_SEQ_NUM + 1);

  }
  memset(buf, '\0', sizeof(buf));
  //recvfrom(sockfd, &recv_packet, sizeof(buf), 0, (struct sockaddr*)&addr, &addr_len);
  //std::cout <<buf<< std::endl;
  close(sockfd);

  //third, finalize





  return 0;
}
