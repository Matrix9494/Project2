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
char ALLFILE[400000][512];
int ALLENGTH[400000];
int ACKLIST[400000] ;
bool ackforfin = 0;
std::chrono::steady_clock::time_point start_time;
chrono::steady_clock::time_point current_time(){
  return chrono::steady_clock::now();
}
int time_elapsed(chrono::steady_clock::time_point time_out){
  return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - time_out).count();
}

int main(int argc, char ** argv)
{
  for (int i = 0;i<100000; i++)
  {
    ALLENGTH[i] = 0;
    ACKLIST[i] = 0;
  }
  if (argc != 4)
  {
    std::cerr<<"ERROR:MUST BE 4 ARGS";
    return 0;
  }

  const char* hostname_or_ip = argv[1];
  int port = atoi(argv[2]);
  const char* filename = argv[3];
  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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

  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = RETRANS_TIMEOUT; // 500ms
  if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) < 0) {
    cerr << "setsockopt failed\n";
  }
  //first, establish the connection

  char buf[DATA_SIZE] = {0};
  memset(buf, '\0', sizeof(buf));
  CURRENT_ACK_NUM = (recv_packet.head.seq + 1) % (MAX_SEQ_NUM + 1);
  generate_packet(send_packet, CURRENT_SEQ_NUM,CURRENT_ACK_NUM,1,buf, sizeof(buf));
  if (sendto(sockfd, &send_packet, sizeof(send_packet), 0, (struct sockaddr*)&addr, addr_len) == -1)
  {
    perror("send error");
    return 1;
  }
  std::cout<<"SEND "<<send_packet.head.seq<<" "<<send_packet.head.ack<<" 0 0 "<<std::endl;

  cout << "first hand "<<CURRENT_SEQ_NUM<<" "<<CURRENT_ACK_NUM<< endl;

  while (!establishedTCP)
  {
    int bytesRec = recvfrom(sockfd, &recv_packet, MSS, 0, (struct sockaddr*)&addr, &addr_len);
    if (bytesRec == -1)
    {

    }
    std::cout<<"RECV "<<recv_packet.head.seq<<" "<<recv_packet.head.ack<<" 0 0 "<<std::endl;
    if ( recv_packet.head.flag == 2 )
    {
      CURRENT_ACK_NUM = recv_packet.head.seq + 1;
      CURRENT_SEQ_NUM = CURRENT_SEQ_NUM + 1;
      establishedTCP = true;
      cout << "finish "<<CURRENT_SEQ_NUM<<" "<<CURRENT_ACK_NUM<< endl;
      break;

    }
  }
  //read the file and save it to ALLFILE
  FILE * file_send = fopen(filename, "rb");
  if(file_send == NULL)
    {
      fprintf(stderr, "ERROR: Can't open the file\n");
      return 4;
    }
  memset(ALLFILE, '\0', sizeof(ALLFILE));
  int ini_file = 0;
  while(1)
  {
    memset(buf, '\0', sizeof(buf));
    int file_not = fread(buf, sizeof(char), sizeof(buf), file_send);
    if(file_not == 0)
    {
      break;
    }

    for (int i = 0; i<file_not; i++)
    {
      ALLFILE[ini_file][i] = buf[i];
    }
    ALLENGTH[ini_file] = file_not;
    ini_file+=1;
  }
  fclose(file_send);
  int file_length = ini_file;
  ini_file = 0;
///timeout/////////////////////

  int cwnd_starting = 0;
  int cwnd_ending = cwnd_starting+10;
  int seq_waitfor_ack = CURRENT_SEQ_NUM;
  while(1)
  {

    //this should be divided into three parts
    //first, we decide the window size, starting and ending
    cwnd_size;




    //second, send all packets in window

    for (int ini_file = cwnd_starting; ini_file<cwnd_ending; ini_file++)
    {
      //determine the packet
      memset(buf, '\0', sizeof(buf));
      for (int i = 0; i<DATA_SIZE; i++)
      {
        buf[i] = ALLFILE[ini_file][i];
      }
      if(ACKLIST[ini_file] == 0)
      {

        if (ini_file == file_length)
        {
          // generate_packet(send_packet, CURRENT_SEQ_NUM,CURRENT_ACK_NUM,4,buf, ALLENGTH[ini_file]);
          // if(sendto(sockfd, &send_packet, sizeof(send_packet), 0,(struct sockaddr *)&addr, addr_len) == -1)
          // {
          //   perror("send error");
          //   return 1;
          // }
          // std::cout<<"SEND "<<send_packet.head.seq<<" "<<send_packet.head.ack<<" 0 0 "<<std::endl;
          //
          // CURRENT_SEQ_NUM = (CURRENT_SEQ_NUM + sizeof(send_packet.data)) % (MAX_SEQ_NUM + 1);
          goto finalize;
        }
        generate_packet(send_packet, CURRENT_SEQ_NUM,CURRENT_ACK_NUM,3,buf, ALLENGTH[ini_file]);
        if(sendto(sockfd, &send_packet, sizeof(send_packet), 0,(struct sockaddr *)&addr, addr_len) == -1)
        {
          perror("send error");
          return 1;
        }
        std::cout<<"SEND "<<send_packet.head.seq<<" "<<send_packet.head.ack<<" 0 0 "<<std::endl;
        CURRENT_SEQ_NUM = (CURRENT_SEQ_NUM + sizeof(send_packet.data)) % (MAX_SEQ_NUM + 1);
        ACKLIST[ini_file] = 1;
      }
    }
    //third, recv from the ack from server
    int bytesRec = recvfrom(sockfd, &recv_packet, MSS, 0, (struct sockaddr*)&addr, &addr_len);
    if (bytesRec == -1)
    {

    }
    std::cout<<"RECV "<<recv_packet.head.seq<<" "<<recv_packet.head.ack<<" 0 0 "<<std::endl;
    if(recv_packet.head.ack == ((seq_waitfor_ack  +DATA_SIZE)% (MAX_SEQ_NUM + 1)))
    {
      CURRENT_ACK_NUM += 1;
      //cout<<"herehere"<<endl;
      cwnd_starting += 1;
      cwnd_ending += 1;
      seq_waitfor_ack = (seq_waitfor_ack  +DATA_SIZE)% (MAX_SEQ_NUM + 1);
      ACKLIST[cwnd_starting] = 2;
    }
  }



  finalize:
  ///here starts Lingxiao/////////////
  packet_head terminate_packet;
  generate_packet_head(terminate_packet, CURRENT_SEQ_NUM,0,4);
  //cout<<CURRENT_SEQ_NUM<<" "<<"normal"<<endl;
  //cout <<CURRENT_SEQ_NUM<<" "<<0<<" "<<" FIN\n";<<endl;
  cout<<"SEND "<<CURRENT_SEQ_NUM<<" "<<0<<" "<<"FIN"<<endl;
  //addr_len may be have issue
  if(sendto(sockfd, &terminate_packet, sizeof(terminate_packet), 0,(struct sockaddr *)&addr, addr_len) == -1)
    {
      perror("FIN send error");
      return 1;
    }
  // the following may change
  CURRENT_SEQ_NUM = (CURRENT_SEQ_NUM + 1) % (MAX_SEQ_NUM + 1);
  // do we need setsockopt?
  //waiting for ACK of FIN, did not consider abort time first
  while (!ackforfin)
  {
    int bytesRec = recvfrom(sockfd, &recv_packet, sizeof(recv_packet), 0, (struct sockaddr*)&addr, &addr_len);
    if (bytesRec==-1)
    {
      //abort time insert here
      // if (time_elapsed(start_time) >= abort_timeout)
      //   {
      //     cerr<<"No packet received in "<<abort_timeout<<" seconds!\nClosing connection. \n\n"<<endl;
      //     close(sockfd);
      //     error_and_exit(ERROR_TIMEOUT);
      //   }
      //   if (EWOULDBLOCK) {
      //       //cerr << "Timed out when waiting for ACK for fin, resending FIN\n";
      //     if (sendto(sockfd, &send_packet_fin, sizeof(send_packet_fin), 0, (struct sockaddr *)&serverAddr, (socklen_t)sizeof(serverAddr)) == -1) {
      //     perror("send error");
      //     return 1;
      //     }
      //   }
      //   else {
      //     perror("Error while listening for ACK");
      //     return 1;
      //   }
    }

    else
    {
          // check the second part
        if ((recv_packet.head.flag==5)&&(recv_packet.head.seq==CURRENT_ACK_NUM))
      {
      cout<<"Recived ACK for FIN"<<endl;
      CURRENT_ACK_NUM = (CURRENT_ACK_NUM + 1) % (MAX_SEQ_NUM + 1);
      ackforfin=1;
      }
    }

  }
  //waiting for FIN from server, 2 seconds
  start_time = current_time();
  //setsockopt?
  while (1)
  {
    int bytesRec = recvfrom(sockfd, &recv_packet, sizeof(recv_packet), 0, (struct sockaddr*)&addr, &addr_len);
    if (bytesRec==-1)
    {
      if (time_elapsed(start_time) >= 2)
      {
        cout<<"wait up to 2 seconds, closing connection"<<endl;
        close(sockfd);
        break;
      }
    }
    else
    {
      if (recv_packet.head.flag == 4)
    {
      cout<<"FIN Received"<<endl;
      //FIN recieved, and send ACK
      packet_head fin_ack_packet;
      //may use seq as 0
      generate_packet_head(fin_ack_packet,0,CURRENT_ACK_NUM,5);
      cout<<"SEND"<<0<<" "<<CURRENT_ACK_NUM<<" "<<"ACK|FIN"<<endl;
      if(sendto(sockfd, &fin_ack_packet, sizeof(fin_ack_packet), 0,(struct sockaddr *)&addr, addr_len) == -1)
      {
        //error("ACK for FIN send error");

        return 1;
      }
    }
      else
      {
        cout << "DROP " << recv_packet.head.seq<<" "<<recv_packet.head.ack<<" "<<recv_packet.head.flag<< endl;
      }
      if (time_elapsed(start_time) >= 2)
      {
        cout<<"wait up to 2 seconds, closing connection"<<endl;
        close(sockfd);
        break;
      }
    }
  }
  //end my part




  memset(buf, '\0', sizeof(buf));

  close(sockfd);
  //third, finalize
  return 0;
}
