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
set<packet> rwnd;
uint32_t CURRENT_SEQ_NUM = 4321;
uint32_t CURRENT_ACK_NUM = 0;
bool establishedTCP = 0;
bool startedHandshake = 0;
packet recv_packet;
packet send_packet;
bool ackforfin = 0;
using namespace std;
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
//use to let recvfrom nonblock and retransmission
  // struct timeval timeout;
  // timeout.tv_sec = 0;
  // timeout.tv_usec = RETRANS_TIMEOUT; // 500ms
  // if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) < 0) {
  //   cerr << "setsockopt failed\n";
  // }

  int bbb = sizeof(recv_packet);
  std::cout<<bbb<<std::endl;
  int ccc = 0;



  while(1)
  {
    //This loop is for different client to establish the connection_num
    //first three handshake


    if (!establishedTCP)
    {
      char buf[300] = {0};
      memset(buf, '\0', sizeof(buf));
      //recv_packet.data = (char*)malloc(DATA_SIZE*sizeof(char));
      int bytesRec = recvfrom(sockfd, &recv_packet, MSS, 0, (struct sockaddr*)&addr, &addr_len);
      if (bytesRec == -1)
      {

      }
      std::cout<<"RECV "<<recv_packet.head.seq<<" "<<recv_packet.head.ack<<" 0 0 "<<std::endl;
      startedHandshake = 1;
      if(recv_packet.head.flag == 1)
      {
        CURRENT_ACK_NUM = (recv_packet.head.seq + 1) % (MAX_SEQ_NUM + 1);
        generate_packet(send_packet, CURRENT_SEQ_NUM,CURRENT_ACK_NUM,2,buf, sizeof(buf));
        if (sendto(sockfd, &send_packet, sizeof(send_packet), 0, (struct sockaddr*)&addr, addr_len) == -1)
        {
          perror("send error");
          return 1;
        }
        std::cout<<"SEND "<<send_packet.head.seq<<" "<<send_packet.head.ack<<" 0 0 "<<std::endl;
        //cout << "second hand "<<CURRENT_SEQ_NUM<<" "<<CURRENT_ACK_NUM<< endl;
        CURRENT_SEQ_NUM += 1;
        continue;
      }
      if(recv_packet.head.flag == 3 && startedHandshake)
      {
        establishedTCP = 1;
        //CURRENT_SEQ_NUM = recv_packet.head.ack;
        //CURRENT_ACK_NUM = (recv_packet.head.seq + sizeof(recv_packet.data)) % (MAX_SEQ_NUM + 1);
        cout << "all done "<<CURRENT_SEQ_NUM<<" "<<CURRENT_ACK_NUM<< endl;
        continue;
      }
    }


    //now begin to transmiss file
    ccc+=1;

    std::cout << ccc<< std::endl;
    char *name = new char[100];
    sprintf(name, "%s/%d.txt", FILE_DIR, ccc);
    FILE * file_write = fopen(name, "wb");


    while(1)
    {
      ///timeout////////////

        ////////////////////
      char buf[300] = {0};
      memset(buf, '\0', sizeof(buf));
      //int break_or_not = recv(sockfd, buf, sizeof(buf), 0);
      if (recv_packet.head.flag == 4)
      {
        //cout<<"eee"<<endl;
        //cout<<strlen(recv_packet.data)<<endl;
        //fwrite(recv_packet.data, sizeof(char), recv_packet.head.lendata, file_write);
        fclose(file_write);
        break;
      }
      //everything is OK
      if (CURRENT_ACK_NUM == recv_packet.head.seq)
      {
        fwrite(recv_packet.data, sizeof(char), recv_packet.head.lendata, file_write);
        CURRENT_ACK_NUM = (CURRENT_ACK_NUM + sizeof(recv_packet.data)) % (MAX_SEQ_NUM + 1);
        generate_packet(send_packet, CURRENT_SEQ_NUM,CURRENT_ACK_NUM,3,buf, sizeof(buf));
        CURRENT_SEQ_NUM = (CURRENT_SEQ_NUM + 1) % (MAX_SEQ_NUM + 1);

        if(sendto(sockfd,&send_packet, sizeof(send_packet), 0, (struct sockaddr *)&addr, addr_len) == -1)
        {
            perror("send error");
            return 1;
        }
        std::cout<<"SEND "<<send_packet.head.seq<<" "<<send_packet.head.ack<<" 0 0 "<<std::endl;
      }
      //cout<<sizeof(recv_packet.data)<<endl;

      int break_or_not = recvfrom(sockfd, &recv_packet, MSS, 0, (struct sockaddr*)&addr, &addr_len);
      if (break_or_not == -1)
      {
        if (EWOULDBLOCK)
        {
          continue;
        }
        else
        {
          std::cout <<"error!"<< std::endl;
          break;
        }

      }
      std::cout<<"RECV "<<recv_packet.head.seq<<" "<<recv_packet.head.ack<<" 0 0 "<<std::endl;


    }

    //now finalize the establish


    establishedTCP = 0;
    startedHandshake = 0;


    ackforfin = 0;
    //my part
    //FIN recieved, and send ACK
    packet_head terminate_packet;
    generate_packet_head(terminate_packet,CURRENT_SEQ_NUM,CURRENT_ACK_NUM,5);
    cout << "SEND "<<CURRENT_SEQ_NUM<<" "<<CURRENT_ACK_NUM<<" "<<"ACK|FIN"<<endl;
    if(sendto(sockfd, &terminate_packet, sizeof(terminate_packet), 0,(struct sockaddr *)&addr, addr_len) == -1)
    {
      perror("ACK for FIN send error");
      return 1;
    }
    // send FIN
    packet_head packet_fin;
    generate_packet_head(packet_fin, CURRENT_SEQ_NUM,0,4);
    cout<<"SEND"<<CURRENT_SEQ_NUM<<" "<<0<<" "<<"FIN"<<endl;
    if(sendto(sockfd, &packet_fin, sizeof(packet_fin), 0,(struct sockaddr *)&addr, addr_len) == -1)
    {
      perror("FIN send error");
      return 1;
    }

    //waiting for ACK of FIN, did not consider abort time first
    while (!ackforfin)
    {
    int bytesRec = recvfrom(sockfd, &recv_packet, sizeof(recv_packet), 0, (struct sockaddr*)&addr, &addr_len);
    if (bytesRec==-1)
    {
      //insert abort time here
      // if (time_elapsed(start_time) >= abort_timeout)//abort connection without receiving packets in 10 seconds.
      //   {
      //     break;
      //   }
      //   if (EWOULDBLOCK) {
      //    if (sendto(sockfd, &send_packet_fin, sizeof(send_packet_fin), 0, (struct sockaddr *)&clientAddr, (socklen_t)sizeof(clientAddr)) == -1) {
      //     perror("send error");
      //     return 1;
      //   }
      // }
      // else {
      //   perror("Error while listening for ACK");
      //   return 1;
      // }
    }
    // check the second part
    else
    {
      if ((recv_packet.head.flag==5)&&(recv_packet.head.seq==0))
      {
        cout<<"Recived ACK for FIN"<<endl;
        ackforfin=1;
      }
    }

    }
    cout<<"Closing connection"<<endl;
    close(sockfd);
    //end my part





  }





  return 0;
}
