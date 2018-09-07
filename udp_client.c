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
#include <errno.h>

#define MAXBUFSIZE 1024



int main (int argc, char * argv[])
{

	int nbytes;                             // number of bytes send by sendto()
	int sock;                               //this will be our socket
	char buffer[MAXBUFSIZE],buff[MAXBUFSIZE];

	struct sockaddr_in remote;              //"Internet socket address structure"
	if (argc < 3)
	{
		printf("USAGE:  <server_ip> <server_port>\n");
		exit(1);
	}

	bzero(&remote,sizeof(remote));               //zero the struct
	remote.sin_family = AF_INET;                 //address family
	remote.sin_port = htons(atoi(argv[2]));      //sets port to network byte order
	remote.sin_addr.s_addr = inet_addr(argv[1]); //sets remote IP address
	if ((sock = socket(PF_INET,SOCK_DGRAM,0)) < 0)
	{
		printf("unable to create socket");
	}


	char command[] = "apple";
	int on=1,n=0;
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));	
	FILE *fp;
	const char filename[100]="/home/gautham/Network Systems/P_1/udp/foo1";
	fp=fopen(filename,"r");
	if(fp==NULL)
		perror("fopen:\n");

	while(1){
		
		n=fread(buff,sizeof(char),MAXBUFSIZE,fp);
		if(n==MAXBUFSIZE)
			if(nbytes=sendto(sock, buff,MAXBUFSIZE,0, (struct sockaddr *)&remote, sizeof(remote))<0)
				perror("sendto:");

		if(n<MAXBUFSIZE){
			printf("Sending Last Packet \n");
			if(nbytes=sendto(sock, buff,n,0, (struct sockaddr *)&remote, sizeof(remote))<0)
				perror("sendto:");
		break;
		}
	}



	struct sockaddr_in from_addr;
	int addr_length = sizeof(struct sockaddr);
	bzero(buffer,sizeof(buffer));
	nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&remote, &addr_length);

	printf("Server says %s\n", buffer);

	close(sock);

}

