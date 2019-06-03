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


//using namespace std;
void signal_handler(int signal)
{
  if (signal == SIGQUIT || signal == SIGTERM)
  {
    exit(0);
  }

}
void single_task(int clientSockfd, int thread_id, char * dir_path);

int
main(int argc, char ** argv)
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
  //char * FILE_DIR = temp_input;

  char *FILE_DIR = new char[100];
  sprintf(FILE_DIR, ".%s", temp_input);
  //printf("%s", FILE_DIR);
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
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // allow others to reuse the address
  int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
  {
    perror("setsockopt");
    return 1;
  }

  // bind address to socket
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);     // short, network byte order
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

  if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
  {
    perror("bind");
    return 2;
  }

  // set socket to listen status
  if (listen(sockfd, 1) == -1)
  {
    perror("listen");
    return 3;
  }
  int ccc = 0;
  // accept a new connection

  while (1)
  {
    ccc = ccc + 1;

    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

    if (clientSockfd == -1) {
      perror("accept");
      return 4;
    }

    char ipstr[INET_ADDRSTRLEN] = {'\0'};
    inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));

    std::cout << "Accept a connection from: " << ipstr << ":" <<
      ntohs(clientAddr.sin_port) << std::endl;
    //printf("%s\n", FILE_DIR);
    //printf("%s", FILE_DIR);
    //printf("aaa\n");
    std::thread(single_task,clientSockfd, ccc, FILE_DIR).detach();
  //

  }






/*
  while (!isEnd) {
    memset(buf, '\0', sizeof(buf));

    if (recv(clientSockfd, buf, 20, 0) == -1) {
      perror("recv");
      return 5;
    }

    ss << buf << std::endl;
    std::cout << buf << std::endl;

    if (send(clientSockfd, buf, 20, 0) == -1) {
      perror("send");
      return 6;
    }

    if (ss.str() == "close\n")
      break;

    ss.str("");
  }
*/


  return 0;
}

void single_task (int clientSockfd, int thread_id, char * dir_path)
{
  ////timeout//////////////////
  fd_set active_fd_set;
  struct timeval time_out;
  FD_ZERO(&active_fd_set);
  FD_SET(clientSockfd, &active_fd_set);
  time_out.tv_sec = 1; //set the timeout to 10s
  time_out.tv_usec = 0;
  //////////////////////////
  //printf("%d", clientSockfd);
  // read/write data from/into the connection
  char *name = new char[100];
  sprintf(name, "%s/%d.txt", dir_path, thread_id);
  //printf("%s", name);
  //char *name = "rrr.txt";
  FILE * file_write = fopen(name, "wb");

  char buf[1024] = {0};




  while(1)
  {


    //std::this_thread::sleep_for(std::chrono::seconds(10));
    ///timeout////////////
    int rs = select(clientSockfd+1, &active_fd_set,  NULL, NULL, &time_out);
    printf("%d\n", rs);
    if(rs < 0)
      {
        fprintf(stderr, "ERROR: NOTHING SELECTED\n");
        return;
      }
    else if( rs == 0)
      {

        close(clientSockfd);
        fprintf(stderr, "ERROR: TIMEOUT\n");



        //Create Empty File//////////////
        char emp[6] = "ERROR";
        file_write = fopen(name, "wb");
        fwrite(emp, sizeof(char), sizeof(emp), file_write);
        fclose(file_write);
        close(clientSockfd);
        return;
      }
      ////////////////////






    char buf[1024] = {0};
    memset(buf, '\0', sizeof(buf));
    int break_or_not = recv(clientSockfd, buf, sizeof(buf), 0);
    if (break_or_not == 0)
    {
      fclose(file_write);
      //printf("aaa\n");
      break;
    }
    fwrite(buf, sizeof(char), break_or_not, file_write);
  }
  close(clientSockfd);
  return ;
}
