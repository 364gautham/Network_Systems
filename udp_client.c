// TODO : Comments and Documentation of usage , Makefile
/********************************************************************************************************
*
* UNIVERSITY OF COLORADO BOULDER
*
* @file UDP_CLIENT.c
* @brief udp CLIENT process which does socket udp communication
*
* @author  Gautham K A
* @date  9/21/2018
*
********************************************************************************************************/


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
//packet structure for data transfer
typedef struct packet{
	uint16_t seq_num;
	uint16_t ack;
	char buff[MAXBUFSIZE];
}packet_t;

struct timeval timeout;
//packet structure for ack transfer
typedef struct ack{
	uint16_t seq;
	uint16_t ack;
}ack_pk_t;

/********************************************************************************************************
*Function to get options to perform necesaary actions like get file,
* put file, list files , delete files and exit server process
********************************************************************************************************/

int opt(){
		printf("\n\n****Enter Options as given below***\n");
		printf("1: put 'Filename' \n2: get 'Filename' \n3: ls \n4:delete 'Filename' \n5:exit\n\n");
		memset(inp,0,30);
		char inp1[30];
		memset(inp1,0,30);
		fgets(inp,30,stdin);
		sscanf(inp,"%s",inp1);

		if(strcmp(inp1,"put")==0)
			return 1;
    if(strcmp(inp1,"get")==0)
			return 2;
		if(strcmp(inp1,"ls")==0)
			return 3;
		if(strcmp(inp1,"delete")==0)
			return 4;
		if(strcmp(inp1,"exit")==0)
			return 5;
		return 0;
}
/********************************************************************************************************
*this function needs filename as input and does put the file through
* udp socket using specified address and given port(command line - run time)t
********************************************************************************************************/
void put_file(char* file)
{
	FILE *fp;
	int n,nbytes;
	fp=fopen(file,"r");
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
		//if(strcmp(ready,"ready") != 0)
			//return;
		memset(ready,0,10);

  //packet pointer
	packet_t *packet=(packet_t*)malloc(sizeof(packet_t));
	ack_pk_t *pkt=(ack_pk_t*)malloc(sizeof(ack_pk_t));
	uint16_t num=1,ack=0;
	timeout.tv_sec = 0;
	timeout.tv_usec = 500000;
	if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO , (char *)&timeout,sizeof(timeout)) < 0)
			perror("setsockopt failed : \n");
	int count=0;

	while(siz>0){
		count=0;
		n=fread(packet->buff,sizeof(char),MAXBUFSIZE,fp);
	  printf("Client reading Bytes: %d \n",n);
		packet->seq_num=num;
		packet->ack=1;
		ack=1;
		while(ack<=1){
					pkt->ack=0;pkt->seq=0;
					nbytes=sendto(sock, packet,n+4,0,(struct sockaddr *)&remote, sizeof(remote));
					printf("Client Sending Packet %d Seq Num:%d \n",nbytes,packet->seq_num);
					nbytes=recvfrom(sock,pkt,sizeof(ack_pk_t), 0, (struct sockaddr *)&remote, &remote_length);
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

/********************************************************************************************************
*this function needs filename as input gets that file from server directory through
* udp socket using specified address and given port(command line - run time)
********************************************************************************************************/
void get_file1(char* file){
	int siz;
	recvfrom(sock,&siz,sizeof(siz), 0, (struct sockaddr *)&remote, &remote_length);

	printf("Received file Size %d \n",siz);

	char ready[10];
	memset(ready,0,10);
	recvfrom(sock,ready,sizeof(ready), 0, (struct sockaddr *)&remote, &remote_length);
	//if(strcmp(ready,"ready") != 0)
		//return;
	printf("\nReady \n\n");

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
		volatile uint16_t seq=0,count=0;
		packet_t *pkt1=(packet_t*)malloc(sizeof(packet_t));
		int i=0;
		timeout.tv_sec = 0;
		timeout.tv_usec = 500000;
		if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
				perror("setsockopt failed : \n");
	 while(siz>0){
						packet->ack=0;
						nbytes=recvfrom(sock,packet,sizeof(packet_t), 0, (struct sockaddr*)&remote, &remote_length);
						if(errno==EAGAIN){
							printf("Receive drop %d\n",packet->seq_num);
							//printf("Ack %d\n",packet->ack);
						}
						if(seq<packet->seq_num){
						n=fwrite(packet->buff,sizeof(char),nbytes-4,fp);
						//printf("write bytes: %d and seq_num %d \n",n,packet->seq_num);
						siz=siz-n;
						pkt->seq=packet->seq_num;
						pkt->ack=packet->ack+1;
						sendto(sock,pkt,sizeof(ack_pk_t),0,(struct sockaddr *)&remote, sizeof(remote));
						}
						if(errno!=EAGAIN && seq==packet->seq_num){
	                printf("ack drop  %d oo %d \n",packet->seq_num,nbytes);
	                pkt->seq=packet->seq_num;
	                pkt->ack=2;
	                sendto(sock,pkt,sizeof(ack_pk_t),0,(struct sockaddr *)&remote, sizeof(remote));
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
*this function needs filename as input and sends that filename to get_file1 to fetch file from server
********************************************************************************************************/
void get_file(char *file){
	int option=2;
 	if(sendto(sock, &option,sizeof(option),0, (struct sockaddr *)&remote, sizeof(remote))<0)
		 perror("send to:\n");
   // sending filename to server to fetch the file
  if(sendto(sock, file,sizeof(file),0, (struct sockaddr *)&remote, sizeof(remote))<0)
 			perror("send to:\n");
	printf("Filename : %s\n",file);

	get_file1(file);
}

/********************************************************************************************************
*this function prints put the file list of server directory
********************************************************************************************************/
void list_files()
{
	int option=3;
	if(sendto(sock, &option,sizeof(option),0, (struct sockaddr *)&remote, sizeof(remote))<0)
		 perror("send to:\n");

  char filename[]="ls_server";
	get_file1(filename);
  // print contents of file
 char c;
 FILE *fp;
 fp=fopen(filename,"r");
 if(fp==NULL)
 		perror("fopen: \n");
 c=fgetc(fp);
 while (c != EOF){
  printf ("%c", c);
  c = fgetc(fp);
	}
  fclose(fp);
		remove(filename);
}

/********************************************************************************************************
*this function needs filename as input and removes that file if present in server directory
********************************************************************************************************/
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
	memset(ack,0,10);
	recvfrom(sock,ack,sizeof(ack), 0, (struct sockaddr *)&remote, &remote_length);
	if(strcmp(ack,"Success") == 0)
		printf("Success Deletion\n" );
}

/********************************************************************************************************
*this function performs exit operation of server process.
********************************************************************************************************/
void exit_operation()
{
	int option=5;
	if(sendto(sock, &option,sizeof(option),0, (struct sockaddr *)&remote, sizeof(remote))<0)
		 perror("send to:\n");
}


/********************************************************************************************************
* Main function creates the udp socket descriptor
********************************************************************************************************/
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
	setsockopt(sock,SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));


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
			else{
				if(sendto(sock, &option,sizeof(option),0, (struct sockaddr *)&remote, sizeof(remote))<0)
					 perror("send to:\n");
				printf("\n***Invalid Command***\n");
				//recvfrom(sock,&option,sizeof(option), 0, (struct sockaddr*)&remote, &remote_length);
			}
		}
	close(sock);
}
