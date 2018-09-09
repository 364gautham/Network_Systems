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
int opt(){
		printf("Enter Options as given below\n");
		printf("1 : Put file \n2: Get File \n");

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
	FILE *fp;	char buff[MAXBUFSIZE];
	char filename[100]="/home/gautham/Network Systems/P_1/udp/";
	strcat(filename,file);
	int n,nbytes;
	fp=fopen(filename,"r");
	if(fp==NULL)
		perror("fopen:\n");
 	int option =1;
	if(sendto(sock, &option,sizeof(option),0, (struct sockaddr *)&remote, sizeof(remote))<0)
			perror("send to:\n");
	if(sendto(sock, file,sizeof(file),0, (struct sockaddr *)&remote, sizeof(remote))<0)
			perror("send to:\n");
	while(1){
		bzero(buff,sizeof(buff));
		n=fread(buff,sizeof(char),MAXBUFSIZE,fp);
	//	printf("Client reading Packet: %d \n",n);

		if(n==MAXBUFSIZE){
		nbytes=sendto(sock, buff,MAXBUFSIZE,0,(struct sockaddr *)&remote, sizeof(remote));
		printf("Client Sending Packet: %d \n",nbytes);
		}

		else if(n<MAXBUFSIZE){
			printf("Sending Last Packet \n");
			nbytes=sendto(sock, buff,n,0, (struct sockaddr *)&remote, sizeof(remote));
		  break;
			fclose(fp);
		}

	}
}

void get_file(char *file){
	 char buf[MAXBUFSIZE];
	 int option =2;
 if(sendto(sock, &option,sizeof(option),0, (struct sockaddr *)&remote, sizeof(remote))<0)
		 perror("send to:\n");
   // sending filename to server to fetch the file
	 if(sendto(sock, file,sizeof(file),0, (struct sockaddr *)&remote, sizeof(remote))<0)
 			perror("send to:\n");
	printf("Filename : %s\n",file);

	char filename[100]="~";
	strcat(filename,file);
	FILE *fp;
	//int fd= open(filename,O_RDWR | O_CREAT,0777);
	fp=fopen(filename,"w+");
	if(fp==NULL)
		perror("open:");
	int n=0,nbytes;
	while(1){
		nbytes = recvfrom(sock,buf,sizeof(buf), 0, (struct sockaddr *)&remote, &remote_length);
		printf("NBYTES %d\n",nbytes);

		n=fwrite(buf,sizeof(char),nbytes,fp);
		printf("write bytes %d\n",n);
		if(n<MAXBUFSIZE)
			break;
	}
}
void list_files()
{}
void delete_file(char * file)
{}
void exit_operation()
{}
int main (int argc, char * argv[])
{
	char buffer[MAXBUFSIZE];
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
				recvfrom(sock,&option,sizeof(option), 0, (struct sockaddr *)&remote, &remote_length);

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
			else if(option==4){
				strcpy(file_name,inp);
				sscanf(file_name,"%s %s",file,file);
				// get file from server
				exit_operation();
			}
		}
	close(sock);
}
