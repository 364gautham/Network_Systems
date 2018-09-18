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

typedef struct ack{
	uint16_t seq;
	uint16_t ack;
}ack_pk_t;


void put_in_server(){
  char file[10];int a;
  char ready[10]="ready";
  int siz;
	recvfrom(sock,file,sizeof(file), 0, (struct sockaddr *)&sin1, &remote_length);
  a=recvfrom(sock,&siz,sizeof(siz), 0, (struct sockaddr *)&sin1, &remote_length);

  if(a!=SO_RCVTIMEO)
    sendto(sock,ready,sizeof(ready),0,(struct sockaddr *)&sin1, sizeof(remote));
  else
  {
    strcpy(ready,"notready");
    sendto(sock,ready,sizeof(ready),0,(struct sockaddr *)&sin1, sizeof(remote));
    return;
  }

 	//printf("Filename Recvd: %s\n",file);
  char filename[10]="~";
	strcat(filename,file);
  printf("Filename Recvd: %s\n",filename);
	FILE *fp;
	fp=fopen(filename,"w+");
	if(fp==NULL)
		perror("open:");
	int n=0,nbytes=0,nbyts=1200;
  //packet pointer
  packet_t *packet=(packet_t*)malloc(sizeof(packet_t));
	ack_pk_t *pkt=(ack_pk_t*)malloc(sizeof(ack_pk_t));
  packet->seq_num=0,pkt->seq=1;
  uint8_t seq=1,count=0;
  packet_t *pkt1=(packet_t*)malloc(sizeof(packet_t));

	while(siz>0){
          nbytes=recvfrom(sock,packet,sizeof(packet_t), 0, (struct sockaddr*)&sin1, &remote_length);
          printf("nbytes %d\n",nbytes);
          if(nbyts<1028 && nbyts >0 && nbytes==-1)
          {
            n=fwrite(pkt1->buff,sizeof(char),nbyts-4,fp);
            printf("write break %d \n",n);
            printf("siz %d",siz);
            fclose(fp);
            break;
          }
          nbyts=nbytes;
          if(seq==packet->seq_num)
              printf("drop drop drop drop drop drop \n");
          pkt->seq=packet->seq_num;
          pkt->ack=packet->ack+1;
          sendto(sock,pkt,sizeof(ack_pk_t),0,(struct sockaddr *)&sin1, sizeof(remote));
          if(seq<packet->seq_num){
            n=fwrite(pkt1->buff,sizeof(char),MAXBUFSIZE,fp);
            printf("write %d \n",n);
            siz=siz-n;
          }
          seq=packet->seq_num;
          memcpy(pkt1,packet,sizeof(packet_t));
  }
}






void send_file_server(){
  // char file[20],buff[MAXBUFSIZE];
	// if(recvfrom(sock, file,sizeof(file), 0, (struct sockaddr *)&sin1, &remote_length)<0)
	// 		perror("recv:\n");
  //
  // printf("Filename Rec: %s\n",file);
  //
  // FILE *fp;
  // char filename[100]="/home/gautham/Network Systems/P_1/udp/";
  // strcat(filename,file);
  // int n,nbytes;
  // printf("Filename Rec: %s\n",filename);
  //
  // fp=fopen(filename,"r");
  // if(fp==NULL)
  //   perror("fopen:\n");
  // while(1){
  //   n=fread(buff,sizeof(char),MAXBUFSIZE,fp);
  //   printf("server reading Packet: %d \n",n);
  //
  //   if(n==MAXBUFSIZE){
  //   nbytes=sendto(sock,buff,MAXBUFSIZE,0,(struct sockaddr *)&sin1, sizeof(remote));
  //   printf("server Sending Packet: %d \n",nbytes);
  //   }
  //
  //   else if(n<MAXBUFSIZE){
  //     printf("Sending Last Packet \n");
  //     if(nbytes=sendto(sock, buff,n,0, (struct sockaddr *)&sin1, sizeof(remote))<0)
  //       perror("sendto:");
  //       fclose(fp);
  //       break;
  //   }
  // }


}

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
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 500000;
	if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
			perror("setsockopt failed : \n");

	if (bind(sock, (struct sockaddr *)&sin1, sizeof(sin1)) < 0)
		printf("unable to bind socket\n");

	remote_length = sizeof(remote);
	int option;int a;
  ssize_t m;
 //client put file to server
  while(1){
  recvfrom(sock, &option,sizeof(option), 0, (struct sockaddr *)&sin1,&remote_length);
 	if(option==1){
 		// store file into server
 		put_in_server();
    option =0;
 	}
   else if(option==2){
     send_file_server();
       option =0;
   }
 }
 close(sock);

}
