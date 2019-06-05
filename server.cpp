#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <sstream>


#include <sys/stat.h>
#include <errno.h>
#include <csignal>
#include <netdb.h>
#include "packet.cpp"

//using namespace std;
void signal_handler(int signal)
{
  if (signal == SIGQUIT || signal == SIGTERM)
  {
    exit(0);
  }

}

int main(int argc, char ** argv)
{

  //initial input part
  if (argc != 3)
  {
    std::cerr<<"ERROR:";
    return 0;
  }

  char *temp_input = argv[1];
  int PORT = atoi(temp_input);
  temp_input =  argv[2];

  char *FILE_DIR = new char[100];
  sprintf(FILE_DIR, ".%s", temp_input);

  int ttt = mkdir(FILE_DIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

///wrong signal//////
  signal(SIGQUIT, signal_handler);
  signal(SIGTERM, signal_handler);


///wrong port///////

  int status;
  struct addrinfo hints;
  struct addrinfo *servinfo;  // will point to the results
  memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
  hints.ai_family = AF_INET;     // don't care IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
  hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
  status = getaddrinfo(NULL, argv[1] , &hints, &servinfo);
  if (status != 0)
    {
      fprintf(stderr, "ERROR:%s\n", gai_strerror(status));
      exit(EXIT_FAILURE);
    }






  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  //int sockfd = 0;

  // allow others to reuse the address
  int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
  {
    perror("setsockopt");
    return 1;
  }

  // bind address to socket
  struct sockaddr_in addr;
  socklen_t addr_len = sizeof(struct sockaddr_in);

  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);     // short, network byte order
  //addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

  if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
  {
    perror("bind");
    return 2;
  }


  int ccc = 0;
  // accept a new connection



  //////timeout//////////////////


  // read/write data from/into the connection






  while(1)
  {
    ccc+=1;
    char *name = new char[100];
    sprintf(name, "%s/%d.txt", FILE_DIR, ccc);

    FILE * file_write = fopen(name, "wb");

    packet recv_packet;

    ///timeout////////////

      ////////////////////
    char buf[300] = {0};
    memset(buf, '\0', sizeof(buf));
    //int break_or_not = recv(sockfd, buf, sizeof(buf), 0);
    int break_or_not = recvfrom(sockfd, &recv_packet, MSS, 0, (struct sockaddr*)&addr, &addr_len);
    std::cout << recv_packet.data<< std::endl;
    // if (break_or_not == 0)
    // {
    //
    //   fclose(file_write);
    //   //printf("aaa\n");
    //   break;
    // }
    // if (break_or_not == -1)
    // {
    //   if (EWOULDBLOCK) {
    //     //This is for waiting!
    //     continue;
    //   }
    //   else{
    //     std::cout <<"error!"<< std::endl;
    //     break;
    //   }
    //
    // }

    std::cout << "yes"<< std::endl;
    fwrite(buf, sizeof(char), break_or_not, file_write);





    sendto(sockfd,buf, sizeof(buf), 0, (struct sockaddr *)&addr, addr_len);

    fclose(file_write);
  }

  return 0;
}
