/**
 * Sample code to communicate e7UDP/IP IP core implemnted on FPGA with UDP/IP
 *
 * e-trees.Japan, Inc.
 * 5th March, 2013
 */

#ifndef	__UDPCOMM_H__
#define	__UDPCOMM_H__

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/**
 * Management of UDP Communication
 */
class UDPComm {

private:
  /**
   * socket for UDP communication
   */
  int sock;
  struct sockaddr_in addr;

public:

  /**
   * Constructor: create UDP socket and bind
   * @param ipaddr target IP address to communicate
   * @param port target port number to communicate
   */
  UDPComm(const char *ipaddr, int port);

  /**
   * Constructor: create UDP socket and bind
   * @param in_addr target IP address to communicate as in_addr_t
   * @param port target port number to communicate
   */
  UDPComm(in_addr_t in_addr, int port);

  /**
   * Destructor: close binded UDP socket
   */
  ~UDPComm();
  
  /**
   * Send data to target host
   * @param data pointer for the data to send
   * @param len length of data to send
   */
  int data_send(char *data, int len);

  /**
   * Receive data form target host
   * @param data pointer to store received data to send
   * @param len length of data to receive
   */
  int data_recv(char *data, int len);
  int get_port();

};

#endif	// __UDPCOMM_H__
