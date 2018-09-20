// TODO : Comments and Documentation of usage

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
char inp[30];
int sock;                               //this will be our socket
struct sockaddr_in remote;              //"Internet socket address structure"
int remote_length;
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

int opt(){
		printf("Enter Options as given below\n");
		printf("1: Put 'Filename' \n2: Get 'Filename' \n3: ls \n4:delete 'Filename' \n5:exit\n");

		fgets(inp,30,stdin);
    if(inp[0]=='g')
			return 2;
		if(inp[0]=='p')
			return 1;
		if(inp[0]=='l')
			return 3;
		if(inp[0]=='d')
			return 4;
		if(inp[0]=='e')
			return 5;
		return 0;
}
// put file to server function
void put_file(char* file)
{
	FILE *fp;
	char filename[100]="/home/gautham/Network Systems/P_1/udp/";
	strcat(filename,file);
	int n,nbytes;
	fp=fopen(filename,"r");
	if(fp==NULL)
		perror("fopen:\n");

	//seek file size
	fseek(fp,0L,SEEK_END);
	int siz = ftell(fp);
	fseek(fp,0L,SEEK_SET);
	printf("file size %d \n",siz);
 	int option =1;
 	char ready[10];

		if(sendto(sock, &option,sizeof(option),0, (struct sockaddr *)&remote, sizeof(remote))<0)
				perror("send to:\n");
		if(sendto(sock,file,sizeof(file),0, (struct sockaddr *)&remote, sizeof(remote))<0)
				perror("send to:\n");
	  if(sendto(sock, &siz,sizeof(siz),0, (struct sockaddr *)&remote, sizeof(remote))<0)
				perror("send to:\n");
		recvfrom(sock,ready,sizeof(ready), 0, (struct sockaddr *)&remote, &remote_length);
		printf("ready %s\n",ready);
		if(strcmp(ready,"ready") != 0)
			return;

  //packet pointer
	packet_t *packet=(packet_t*)malloc(sizeof(packet_t));
	ack_pk_t *pkt=(ack_pk_t*)malloc(sizeof(ack_pk_t));
	uint16_t num=1,ack=0;

	while(siz>0){

		n=fread(packet->buff,sizeof(char),MAXBUFSIZE,fp);
	  printf("Client reading Packet: %d \n",n);
		packet->seq_num=num;
		packet->ack=1;
		ack=1;
		while(ack<=1){
					nbytes=sendto(sock, packet,n+4,0,(struct sockaddr *)&remote, sizeof(remote));
					printf("Client Sending Packet: %d \n",nbytes);
					//wait for ack
					printf("wait ack\n");

					nbytes=recvfrom(sock,pkt,sizeof(ack_pk_t), 0, (struct sockaddr *)&remote, &remote_length);
					ack=pkt->ack;
					printf("ack%d\n",ack);
		}
		num++;
		siz=siz-n;
	}
	fclose(fp);
}



void get_file11(char* file){
	int siz;
	recvfrom(sock,&siz,sizeof(siz), 0, (struct sockaddr *)&remote, &remote_length);

	char ready[10];
	recvfrom(sock,ready,sizeof(ready), 0, (struct sockaddr *)&remote, &remote_length);
	if(strcmp(ready,"ready") != 0)
		return;
	char filename[100]="~";
	strcat(filename,file);
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
	          nbytes=recvfrom(sock,packet,sizeof(packet_t), 0, (struct sockaddr*)&remote, &remote_length);
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
	          sendto(sock,pkt,sizeof(ack_pk_t),0,(struct sockaddr *)&remote, sizeof(remote));
	          if(seq<packet->seq_num){
	            n=fwrite(pkt1->buff,sizeof(char),MAXBUFSIZE,fp);
	            printf("write %d \n",n);
	            siz=siz-n;
	          }
	          seq=packet->seq_num;
	          memcpy(pkt1,packet,sizeof(packet_t));
	  }
}

void get_file(char *file){
	int option =2;
 	if(sendto(sock, &option,sizeof(option),0, (struct sockaddr *)&remote, sizeof(remote))<0)
		 perror("send to:\n");
   // sending filename to server to fetch the file
  if(sendto(sock, file,sizeof(file),0, (struct sockaddr *)&remote, sizeof(remote))<0)
 			perror("send to:\n");
	printf("Filename : %s\n",file);

	get_file11(file);
}

void list_files()
{
	int option=3;
	if(sendto(sock, &option,sizeof(option),0, (struct sockaddr *)&remote, sizeof(remote))<0)
		 perror("send to:\n");

  char filename[]="ls_server";
	get_file11(filename);
  // print contents of file

}
void delete_file(char* file)
{
	int option=4,n;
	if(sendto(sock, &option,sizeof(option),0, (struct sockaddr *)&remote, sizeof(remote))<0)
		 perror("send to:\n");
	//send file_name
	n=sendto(sock,file,20,0, (struct sockaddr *)&remote, sizeof(remote));
	printf("n %d \n",n);
	// get confirmation
	char ack[10];
	recvfrom(sock,ack,sizeof(ack), 0, (struct sockaddr *)&remote, &remote_length);
	if(strcmp(ack,"Success") == 0)
		printf("Success Deletion\n" );

}
void exit_operation()
{
	int option=5;
	if(sendto(sock, &option,sizeof(option),0, (struct sockaddr *)&remote, sizeof(remote))<0)
		 perror("send to:\n");
}


int main (int argc, char * argv[])
{
	if (argc < 3)
	{
		printf("USAGE:  <server_ip> <server_port>\n");
		exit(1);
	}
  remote_length=sizeof(remote);
	bzero(&remote,sizeof(remote));               //zero the struct
	remote.sin_family = AF_INET;                 //address family
	remote.sin_port = htons(atoi(argv[2]));      //sets port to network byte order
	remote.sin_addr.s_addr = inet_addr(argv[1]); //sets remote IP address
	if ((sock = socket(PF_INET,SOCK_DGRAM,0)) < 0)
		printf("unable to create socket");
	int on=1;
	struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 500000;
	setsockopt(sock,SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
	if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO , (char *)&timeout,sizeof(timeout)) < 0)
			perror("setsockopt failed : \n");

 // get option from user
  char file_name[40];char file[20];
	memset(file_name, '\0', sizeof(file_name));

  int option;
	while(1){
			option = opt();
			if(option==1){
				strcpy(file_name,inp);
		    sscanf(file_name,"%s %s",file,file);
		 		// put file to server
				put_file(file);
			}

			else if(option==2){
				strcpy(file_name,inp);
				sscanf(file_name,"%s %s",file,file);
				// get file from server
				get_file(file);
			}
			else if(option==3){
				list_files();
			}
			else if(option==4){
				strcpy(file_name,inp);
				sscanf(file_name,"%s %s",file,file);
				// get file from server
				delete_file(file);
			}
			else if(option==5){
				strcpy(file_name,inp);
				sscanf(file_name,"%s %s",file,file);
				// get file from server
				exit_operation();
			}
		}
	close(sock);
}
