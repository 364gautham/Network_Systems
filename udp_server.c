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

int main (int argc, char * argv[] )
{


	int sock;                           //This will be our socket
	struct sockaddr_in sin, remote;     //"Internet socket address structure"
	unsigned int remote_length;         //length of the sockaddr_in structure
	int nbytes;                        //number of bytes we receive in our message
	char buffer[MAXBUFSIZE],buf[MAXBUFSIZE];             //a buffer to store our received message
	
	if (argc != 2)
	{
		printf ("USAGE:  <port>\n");
		exit(1);
	}

	bzero(&sin,sizeof(sin));                    //zero the struct
	sin.sin_family = AF_INET;                   //address family
	sin.sin_port = htons(atoi(argv[1]));        //htons() sets the port # to network byte order
	sin.sin_addr.s_addr = INADDR_ANY;           //supplies the IP address of the local machine

	if ((sock = socket(PF_INET,SOCK_DGRAM,0)) < 0)
		printf("unable to create socket");

	if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		printf("unable to bind socket\n");
	
	remote_length = sizeof(remote);
	bzero(buffer,sizeof(buffer));
	bzero(buf,sizeof(buf));
	
	const char filename[100]="/home/gautham/Network Systems/P_1/udp/foo11.txt";
	FILE *fp;
	int fd= open(filename,O_RDWR | O_CREAT,0777);
	fp=fopen(filename,"w");
	if(fp==NULL)
		perror("open:");
	int n=0;
	while(1){
		nbytes = recvfrom(sock, buf,sizeof(buf), 0, (struct sockaddr *)&sin, &remote_length);
		printf("NBYTES %d\n",nbytes);
		
		n=fwrite(buf,sizeof(char),nbytes,fp);
		printf("write bytes %d\n",n);
		if(n<MAXBUFSIZE)
			break;
	}
	char msg[] = "orange";
	nbytes = sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&sin, remote_length);


	close(sock);
}

