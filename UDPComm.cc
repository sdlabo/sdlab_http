/**
 * Sample code to communicate e7UDP/IP IP core implemnted on FPGA with UDP/IP
 *
 * e-trees.Japan, Inc.
 * 5th March, 2013
 */

#include <UDPComm.h>

UDPComm::UDPComm(const char*ipaddr, int port)
{
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ipaddr);
  bind(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
}

UDPComm::UDPComm(in_addr_t in_addr, int port)
{
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = in_addr,
  bind(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
}

UDPComm::~UDPComm()
{
  close(sock);
}
  
int UDPComm::data_send(char *data, int len)
{
  return sendto(sock, data, len, 0, (struct sockaddr *)&addr, sizeof(addr));
}

int UDPComm::data_recv(char *data, int len)
{
  return recv(sock, data, len, 0);
}

int UDPComm::get_port()
{
  return ntohs(addr.sin_port);
}
