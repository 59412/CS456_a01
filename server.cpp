#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

using namespace std;
//g++ -std=c++11 taskname.cpp -o
void request_handler(int server_UDP, char* req_code)
{
    int n;
    int r_port;
    
    //Try to receive requests from clients using UDP socket
    char buffer[1024] = {0};

    //printf("Waiting for new requests\n");
    struct sockaddr_in client_UDP = {0};
    socklen_t client_socket_len = sizeof(struct sockaddr_in);
    n = recvfrom(server_UDP,buffer,strlen(req_code),0, (struct sockaddr *)&client_UDP, &client_socket_len);
    if(n < 0)
    {
      printf("Unable to receive UDP request from client!\n");
      return;
    }
    
    string req_c(buffer);
    
    //printf("Request Code %s received!\n",buffer);
    
    //Confirm if the request code is correct
    if(req_c != req_code)
    {
      printf("Incorrect Request Code!\n");
      return;
    }
    
    //Establish TCP connection
    bzero(buffer,1024);
        
    struct sockaddr_in server_TCP = {0};
    //Initialize TCP socket
    int socketTCP = socket(AF_INET, SOCK_STREAM,0);
    
    server_TCP.sin_family = AF_INET;
	  server_TCP.sin_addr.s_addr = htonl(INADDR_ANY);
 
    //Look for the first available port number after the random generated number
    bool found_port = false;
    int rand_port_number = rand() % 50000 + 1025;
    
    for(int i = rand_port_number; i < 65535; i++)
    {
      server_TCP.sin_port = htons(i);
      if(bind(socketTCP, (struct sockaddr*)&server_TCP, sizeof(server_TCP))==0)
      {
        found_port = true; 
        r_port = i;
        break;
      }
    }
    
    //No port available, stop the request
    if(!found_port)
    {
      printf("Failed to find available port number on server for r_port!\n");
      return;
    }

    //Confirm the port chosen for TCP connection
    //printf("r_port: %d choosen!\n Waiting for confirmation...\n",r_port);
    
    //Send r_port to client
    string s = std::to_string(r_port);
    char const *port_res = s.c_str();
    
    bzero(buffer,1024);
    n = sendto(server_UDP,port_res,strlen(port_res),0,(struct sockaddr*)&client_UDP,client_socket_len);
    if(n < 0)
    {
      printf("Failed to send client r_port!\n");
      return;
    }
    
    //Receive the confirmation from client, '1' means received, '0' means not
    bzero(buffer,1024);
    n = recvfrom(server_UDP,buffer,1,0, (struct sockaddr *)&client_UDP, &client_socket_len);
    
    char r_port_res = '0';
    r_port_res = buffer[0];
    if(r_port_res != '1')
    {
      printf("Client failed to reply for r_port!\n");
      return;
    }
    
    //TCP port successfully constructed, officially output TCP port number
    printf("SERVER_TCP_PORT=%d\n", r_port);
    
    //Start to accept requests for TCP connections
    n = listen(socketTCP,5);
    if(n < 0)
    {
      printf("Failed listening!\n");
      return;
    }

    int newsockfd;
    newsockfd = accept(socketTCP,NULL,NULL);
    if (newsockfd < 0) 
    {
      printf("An error happened while accepting!\n");
      printf("%s\n",strerror(errno));
      return;
    }
    
    //printf("TCP permission given\n");
    
    //Receive msg from client
    bzero(buffer,1024);
    n = read(newsockfd,buffer,1024);
    if(n < 0)
    {
      printf("Failed to receive the message from client!\n");
      return;
    }
    
    //Output msg received using the new TCP connection
    printf("SERVER_RCV_MSG = %s\n",buffer);
    int size_msg = strlen(buffer);
    
    //Reverse the msg
    for(int i = 0; i < size_msg/2; i++)
    {
      char temp = buffer[i];
      buffer[i] = buffer[size_msg - 1 - i];
      buffer[size_msg - 1 - i] = temp;
    }
  
    //Send msg to client through TCP conenction
    write(newsockfd,buffer,strlen(buffer));//change newsockfd
    bzero(buffer,1024);
    //Requested handled, exiting. Wait for next request
    printf("Request Handled, disconnecting...\n");
    printf("***************************************************\n");
    
    //Close TCP socket
    close(newsockfd);
    close(socketTCP);
    
}

int main(int argc, char *argv[])
{
  //Initializing Variables
  char buffer[1024];
  char* req_code;
  int n_port,socketUDP,socketTCP;
  struct sockaddr_in server_UDP = {0};
  
  req_code = argv[1];
  
  //Construct initial UDP for server
  socketUDP = socket(AF_INET,SOCK_DGRAM,0);
  if(socketUDP < 0)
  {
    printf("Failed to construct server UDP socket\n");
    return -1;
  }
  
  server_UDP.sin_family = AF_INET;
	server_UDP.sin_addr.s_addr = htonl(INADDR_ANY);
 
  //Choose n_port from 1025 to max port number
  bool found_port = false;
  for(int i = 1025; i < 65535; i++)
  {
    server_UDP.sin_port = htons(i);
    if(bind(socketUDP, (struct sockaddr*) &server_UDP, sizeof(server_UDP)) >= 0)
    {
      found_port = true; 
      n_port = i;
      break;
    }
  }
  
  //No port is available, stop the program
  if(!found_port)
  {
    printf("Failed to find available port number on server for n_port!\n");
    return -1;
  }

  printf("SERVER_PORT=%d\n", n_port);
    
  //UDP socket for server constructed succesffully and starts listening
  while(1)
  {
      // while loop for handling continous requests from clientss
      request_handler(socketUDP,req_code);
  }
}
