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

void put_in_server(){

  char file[20],buf[MAXBUFSIZE];
	if(recvfrom(sock, file,sizeof(file), 0, (struct sockaddr *)&sin1, &remote_length)<0)
			perror("recv:\n");
 	printf("Filename Recvd: %s\n",file);
	char filename[100]="~";
	strcat(filename,file);
  printf("Filename Recvd: %s\n",filename);
	FILE *fp;
	//int fd= open(filename,O_RDWR | O_CREAT,0777);
	fp=fopen(filename,"w+");
	if(fp==NULL)
		perror("open:");
	int n=0,nbytes;
	while(1){
		nbytes = recvfrom(sock, buf,sizeof(buf), 0, (struct sockaddr *)&sin1, &remote_length);
		printf("NBYTES %d\n",nbytes);

		n=fwrite(buf,sizeof(char),nbytes,fp);
		printf("write bytes %d\n",n);
		if(n<MAXBUFSIZE)
			break;
	}
}

void send_file_server(){
  char file[20],buff[MAXBUFSIZE];
	if(recvfrom(sock, file,sizeof(file), 0, (struct sockaddr *)&sin1, &remote_length)<0)
			perror("recv:\n");

  printf("Filename Rec: %s\n",file);

  FILE *fp;
  char filename[100]="/home/gautham/Network Systems/P_1/udp/";
  strcat(filename,file);
  int n,nbytes;
  printf("Filename Rec: %s\n",filename);

  fp=fopen(filename,"r");
  if(fp==NULL)
    perror("fopen:\n");
  while(1){
    n=fread(buff,sizeof(char),MAXBUFSIZE,fp);
    printf("server reading Packet: %d \n",n);

    if(n==MAXBUFSIZE){
    nbytes=sendto(sock,buff,MAXBUFSIZE,0,(struct sockaddr *)&sin1, sizeof(remote));
    printf("server Sending Packet: %d \n",nbytes);
    }

    else if(n<MAXBUFSIZE){
      printf("Sending Last Packet \n");
      if(nbytes=sendto(sock, buff,n,0, (struct sockaddr *)&sin1, sizeof(remote))<0)
        perror("sendto:");
    break;
    }
  }


}

int main (int argc, char * argv[] )
{
	int nbytes;                        //number of bytes we receive in our message
	char buffer[MAXBUFSIZE];             //a buffer to store our received message
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
	bzero(buffer,sizeof(buffer));

	int option;
 //client put file to server
  recvfrom(sock, &option,sizeof(option), 0, (struct sockaddr *)&sin1, &remote_length);
	if(option==1){
		// store file into server
		put_in_server();
	}
  else if(option==2){
    send_file_server();
  }
	close(sock);
}
