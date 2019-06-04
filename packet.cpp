#include <string.h>
#include <thread>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <fstream>
#include <cassert>
#include <sys/time.h>
#include <vector>
#include <set>

using namespace std;

const uint32_t MAX_SEQ_NUM = 25600;
const uint16_t MAX_PKT_LEN = 524;
const uint16_t INIT_CWND_SIZE = 512;
const uint16_t INIT_SS_THRESH = 5120;
const uint16_t MIN_SS_THRESH = INIT_CWND_SIZE;
const uint16_t CWND_MAX_SIZE = 10240;
const uint32_t RETRANS_TIMEOUT = 500000;
const int abort_timeout = 10;


string flag_in_str = "";
#define SYN 0x2
#define ACK 0x4
#define FIN 0x1
#define SYN_ACK SYN|ACK
#define ACK_FIN ACK|FIN

#define MSS 524
#define DATA_SIZE 512



struct header
{
  uint32_t seq;
  uint32_t ack;
  uint16_t flag;
};

struct packet
{
  header header;
  char data[DATA_SIZE];
};
