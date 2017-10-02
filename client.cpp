#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[])
{
  //Initialize all variables and sockets
  int socketUDP,socketTCP,n_port,req_cpde,n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  string server_address;
  n_port = atoi(argv[2]);
  int req_code = atoi(argv[3]);
  char* req_c = argv[3];
  char* msg = argv[4];
  
  char buffer[1024];
  
  //Construct UDP socket for client
  socketUDP = socket(AF_INET,SOCK_DGRAM,0);
  if(socketUDP < 0)
  {
    printf("Failed to build client UDP socket!\n");
    return 0;
  }
  
  //Get information about the server and store them for further usage
  server = gethostbyname(argv[1]);
  if (server == NULL) 
  {
    printf("Server not found!\n");
    exit(0);
  }
  printf("Server found!\n");
  
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
  serv_addr.sin_port = htons(n_port);
  
  //Send request code to server for further communcation, through UDP sockets
  printf("Sending request code!\n");
  n = sendto(socketUDP,req_c,strlen(req_c),0,(struct sockaddr*) &serv_addr,sizeof(struct sockaddr));
  if(n < 0)
  {
    printf("Failed to send request number to server!\n");
  }
  
  //Receivd r_port from server using UDP sockets
  bzero(buffer,1024);
  printf("waiting for r_port..\n");
  n = recvfrom(socketUDP,buffer,1023,0, NULL,NULL);
  if(n < 0)
  {
    printf("Error receiving r_port!\n");
  }
  string rport(buffer);
  int r_port = stoi(rport);
  
  //Confirm that client received the r_port number
  printf("r_port %d received\nSending Confirmation\n",r_port);
  
  //Reply to server for the confirmation that r_port is received
  char resr[1];
  resr[0] = '1';
  n = sendto(socketUDP,resr,strlen(resr),0,(struct sockaddr*) &serv_addr,sizeof(struct sockaddr));
  if(n < 0)
  {
    printf("Failed to confirm r_port to server!\n");
  }
  
  //Request already verified, close UDP socket and use TCP connection for further communcation
  close(socketUDP);
  printf("Client UDP disconnected...\n");
  
  //Renew server information(port number)
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
  serv_addr.sin_port = htons(r_port);
  
  //Construct TCP socket for client
  socketTCP = socket(AF_INET,SOCK_STREAM,0);
  if(socketTCP < 0)
  {
    printf("Failed to build client TCP socket!\n");
    return 0;
  }
  
  //Try to connect to server, timeout after 10k attempts
  int limit = 10000;
  while (connect(socketTCP,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
  {
    printf("Attmepting to establish TCP connection!\n");
    if(limit-- == 0)
    {
      printf("TCP connection Timeout!\n");
      return 0;
    }
  }
  
  //TCP connection successfully established
  printf("TCP connection established!\n");
  
  //Send msg to server through TCP connection
  bzero(buffer,1024);
  n = write(socketTCP,msg,strlen(msg));
  if(n < 0)
  {
    printf("Failed to write message to socket!\n");
  }
  
  //Read processed msg from server using TCP connection
  bzero(buffer,1024);
  n = read(socketTCP,buffer,1023);
  if(n < 0)
  {
    printf("Failed to read processed message from socket!\n");
  }
  
  //Output Result
  printf("Result: \n %s\n",buffer);
  
  //Disconnect after quest is finished
  close(socketTCP);
  return 0;
}
