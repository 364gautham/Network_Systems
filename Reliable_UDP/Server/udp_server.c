/********************************************************************************************************
*
* UNIVERSITY OF COLORADO BOULDER
*
* @file UDP_SERVER.c
* @brief udp server process which does socket udp communication with relaible transfer
* Reliable stop and wait is being implemented for the communication
*
* @author  Gautham K A
* @date  9/21/2018
*
********************************************************************************************************/



#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

// size of data sent in each packet
#define MAXBUFSIZE 1024
int sock;                           //This will be our socket
struct sockaddr_in sin1,remote;     //"Internet socket address structure"
unsigned int remote_length;         //length of the sockaddr_in structure
//packet structure
typedef struct packet{
  uint16_t seq_num;
	uint16_t ack;
	char buff[MAXBUFSIZE];
}packet_t;
// ack packet structure
typedef struct ack{
	uint16_t seq;
	uint16_t ack;
}ack_pk_t;
struct timeval timeout;
/********************************************************************************************************
*This function is used to store file transferred from client to its local directory
* UDP Sockets are used with reliabile communication for drop packets issue and used file write for writing
********************************************************************************************************/

void put_in_server(){
  char file[10];int a;
  char ready[10]="ready";
  int siz;
	recvfrom(sock,file,sizeof(file), 0, (struct sockaddr *)&sin1, &remote_length);
  a=recvfrom(sock,&siz,sizeof(siz), 0, (struct sockaddr *)&sin1, &remote_length);
  memset(ready,0,10);
  if(a>0){
    strcpy(ready,"ready");
    sendto(sock,ready,sizeof(ready),0,(struct sockaddr *)&sin1, sizeof(remote));
    printf("Server Sent Ready \n");
    printf("Filename Recvd:%s\n",file);
  }
  else{
    strcpy(ready,"notready");
    sendto(sock,ready,sizeof(ready),0,(struct sockaddr *)&sin1, sizeof(remote));
    printf("Server Sent Not Ready \n");
    printf("Exiting \n");
    return;
  }

	FILE *fp;
	fp=fopen(file,"w+");
	if(fp==NULL)
		perror("open:");
	int n=0,nbytes=0;
  //packet pointer
  packet_t *packet=(packet_t*)malloc(sizeof(packet_t));
	ack_pk_t *pkt=(ack_pk_t*)malloc(sizeof(ack_pk_t));
  packet->seq_num=0,pkt->seq=1;packet->ack=0;
  //change seq
  volatile uint16_t seq=0;
  packet_t *pkt1=(packet_t*)malloc(sizeof(packet_t));
  int i=0;int er;
  // timeout for socket for reliable transfer implementation
  timeout.tv_sec = 0;
  timeout.tv_usec = 500000;
  if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
      perror("setsockopt failed : \n");
	while(siz>0){
          pkt->ack=0;
          nbytes=recvfrom(sock,packet,sizeof(packet_t), 0, (struct sockaddr*)&sin1, &remote_length);
          if(errno==EAGAIN){
            printf("Receive drop %d nbytes %d\n",packet->seq_num,nbytes);
          }

          if(seq<packet->seq_num){
          n=fwrite(packet->buff,sizeof(char),nbytes-4,fp);
          //printf("write bytes: %d and seq_num %d \n",n,packet->seq_num);
          siz=siz-n;
          pkt->seq=packet->seq_num;
          pkt->ack=packet->ack+1;
          sendto(sock,pkt,sizeof(ack_pk_t),0,(struct sockaddr *)&sin1, sizeof(remote));
          }
          if(errno!=EAGAIN && seq==packet->seq_num){
                printf("ack drop  %d oo %d \n",packet->seq_num,nbytes);
                pkt->seq=packet->seq_num;
                pkt->ack=2;
                sendto(sock,pkt,sizeof(ack_pk_t),0,(struct sockaddr *)&sin1, sizeof(remote));
          }
          errno=0;
          seq=packet->seq_num;
  }
  printf("write Last packet data:%d  and seq_num: %d\n",n,seq);
  fclose(fp);
  free(packet);free(pkt);free(pkt1);
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
      perror("setsockopt failed : \n");
}

/********************************************************************************************************
*This function takes filename as inout and sends that file if present in local directory
* to client connected through UDP socket connection. Reliable tarnsfer of UDP protocol.
********************************************************************************************************/

// function similar to put file in client
void send_file(char* file)
{
  FILE *fp;

	int n,nbytes;
	fp=fopen(file,"r");
	if(fp==NULL)
		perror("fopen:\n");

	//seek file size
	int siz;
  fseek(fp,0L,SEEK_END);
  siz = ftell(fp);
	fseek(fp,0L,SEEK_SET);
	printf("file size %d \n",siz);
 	int option =1;
 	char ready[10]="ready";

	  if(sendto(sock, &siz,sizeof(siz),0, (struct sockaddr *)&sin1, sizeof(remote))<0)
				perror("send to:\n");
    if(sendto(sock, ready,sizeof(ready),0, (struct sockaddr *)&sin1, sizeof(remote))<0)
        perror("send to:\n");


    //packet pointer
  	packet_t *packet=(packet_t*)malloc(sizeof(packet_t));
  	ack_pk_t *pkt=(ack_pk_t*)malloc(sizeof(ack_pk_t));
  	uint16_t num=1,ack=0;
  	timeout.tv_sec = 0;
  	timeout.tv_usec = 500000;
  	if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO , (char *)&timeout,sizeof(timeout)) < 0)
  			perror("setsockopt failed : \n");
  	while(siz>0){
  		n=fread(packet->buff,sizeof(char),MAXBUFSIZE,fp);
  	  printf("Server reading Bytes: %d \n",n);
  		packet->seq_num=num;
  		packet->ack=1;
  		ack=1;
  		while(ack<=1){
  					pkt->ack=0;pkt->seq=0;
  					nbytes=sendto(sock, packet,n+4,0,(struct sockaddr *)&sin1, sizeof(remote));
  					printf("Server Sending Packet %d Seq Num:%d \n",nbytes,packet->seq_num);
  					nbytes=recvfrom(sock,pkt,sizeof(ack_pk_t), 0, (struct sockaddr *)&sin1, &remote_length);
  					ack=pkt->ack;
  					printf("ack value %d",ack);
  					printf(" for Sequence Number: %d \n",pkt->seq);
  		}
  		num++;
  		siz=siz-n;

  	}
  	fclose(fp);
  	free(pkt);free(packet);
  	timeout.tv_sec = 0;
  	timeout.tv_usec = 0;
  	if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO , (char *)&timeout,sizeof(timeout)) < 0)
  			perror("setsockopt failed : \n");
}
// wrapper around send file : used to utiised list files implementation

void send_file_server(){
  char file[20];
	if(recvfrom(sock, file,sizeof(file), 0, (struct sockaddr *)&sin1, &remote_length)<0)
			perror("recv:\n");
  printf("Filename Rec: %s\n",file);
  send_file(file);
}

/********************************************************************************************************
*This function is uses system command to execute ls -la command and is redirected its
* output to a file which is transferred to client which implements list files in server function for user
********************************************************************************************************/

void list_files()
{
  char file[10]="ls_file";
  FILE *fp;
  fp=fopen(file,"w+");
	if(fp==NULL)
		perror("fopen:\n");
  system("ls -la> ls_file");
  send_file(file);
  remove(file);
}

// This function deletes the files if present in server directory

void delete_file(){

  //get file_name
  char file[20];
  char ack[10]="Success";
  int n;
  n=recvfrom(sock, file,20, 0, (struct sockaddr *)&sin1, &remote_length);
  printf("n %d\n",n);
  printf("%s\n",file);
  memset(ack,0,10);
  // use remove function to remove file_name
  if(remove(file)==0){
    sendto(sock,ack,sizeof(ack),0,(struct sockaddr *)&sin1, sizeof(remote));
    printf("Deleted Successfully\n");
  }
  else{
    printf("Unable to delete file\n");
    strcpy(ack,"failure");
    sendto(sock,ack,sizeof(ack),0,(struct sockaddr *)&sin1, sizeof(remote));
  }

}

/********************************************************************************************************
* Main function creates the udp socket descriptor and takes required option from client
* and directs to specific functions for socket communication transfer.
********************************************************************************************************/

int main (int argc, char * argv[] )
{
	int nbytes;                        //number of bytes we receive in our message
	if (argc != 2)
	{
		printf ("USAGE:  <port>\n");
		exit(1);
	}
	bzero(&sin1,sizeof(sin1));                    //zero the struct
	sin1.sin_family = AF_INET;                   //address family
	sin1.sin_port = htons(atoi(argv[1]));        //htons() sets the port # to network byte order
	sin1.sin_addr.s_addr = INADDR_ANY;           //supplies the IP address of the local machine

	if ((sock = socket(PF_INET,SOCK_DGRAM,0)) < 0)
		printf("unable to create socket");


	if (bind(sock, (struct sockaddr *)&sin1, sizeof(sin1)) < 0)
		printf("unable to bind socket\n");

	remote_length = sizeof(remote);
	int option;

  while(1){
      printf("\n**Server Waiting for Command**\n");
      recvfrom(sock, &option,sizeof(option), 0, (struct sockaddr *)&sin1,&remote_length);
      if(option==1){
     		// store file into server
     		put_in_server();
     	}
      else if(option==2){
        send_file_server();
      }

      else if(option==3){
        list_files();
      }
      else if(option==4){
        delete_file();
      }
      else if(option==5){
          printf("\n\n**Exiting Server** \n");
          break;
      }
      else{
        //sendto(sock,&option,sizeof(option),0,(struct sockaddr *)&sin1, sizeof(remote));
        printf("command not understood  \n");
      }
  }
 close(sock);
}
