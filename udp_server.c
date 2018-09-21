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

  if(a!=SO_RCVTIMEO){
    sendto(sock,ready,sizeof(ready),0,(struct sockaddr *)&sin1, sizeof(remote));
    printf("Server Sent Ready \n");
    printf("Filename Recvd:%s\n",file);

  }
  else
  {
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
	int n=0,nbytes=0,nbyts=1200;
  //packet pointer
  packet_t *packet=(packet_t*)malloc(sizeof(packet_t));
	ack_pk_t *pkt=(ack_pk_t*)malloc(sizeof(ack_pk_t));
  packet->seq_num=0,pkt->seq=1;packet->ack=0;
  //change seq
  uint16_t seq=0,count=0;
  packet_t *pkt1=(packet_t*)malloc(sizeof(packet_t));
  int i=0;int er;
	while(siz>0){
          // nbytes=recvfrom(sock,packet,sizeof(packet_t), 0, (struct sockaddr*)&sin1, &remote_length);
          // //printf("nbytes %d\n",nbytes);
          // er=errno;
          // if(nbyts<1028 && nbyts >0 && nbytes==-1 )
          // {
          //   n=fwrite(pkt1->buff,sizeof(char),nbyts-4,fp);
          //   printf("write Last packet data:%d  and seq_num: %d\n",n,seq);
          //   fclose(fp);
          //   break;
          // }
          // nbyts=nbytes;
          // if(seq==packet->seq_num){
          //   printf("drop \n");i++;
          //   //if(i==2)
          //   //{break;fclose(fp);}
          // }
          // pkt->seq=packet->seq_num;
          // pkt->ack=packet->ack+1;
          // sendto(sock,pkt,sizeof(ack_pk_t),0,(struct sockaddr *)&sin1, sizeof(remote));
          // if(seq<packet->seq_num){
          //   n=fwrite(pkt1->buff,sizeof(char),MAXBUFSIZE,fp);
          //   printf("write bytes: %d and seq_num %d \n",n,seq);
          //   siz=siz-n;
          // }
          // seq=packet->seq_num;
          // if(er!=ETIMEDOUT)
          // memcpy(pkt1,packet,sizeof(packet_t));
          packet->ack=0;
          nbytes=recvfrom(sock,packet,sizeof(packet_t), 0, (struct sockaddr*)&sin1, &remote_length);
          //printf("nbytes %d\n",nbytes);
          er=errno;
          if(nbytes<1028 && nbytes >0)
          {
            n=fwrite(packet->buff,sizeof(char),nbytes-4,fp);
            printf("write Last packet data:%d  and seq_num: %d\n",n,seq);
            fclose(fp);
            pkt->seq=packet->seq_num;
            pkt->ack=packet->ack+1;
            sendto(sock,pkt,sizeof(ack_pk_t),0,(struct sockaddr *)&sin1, sizeof(remote));

            break;
          }
          nbyts=nbytes;
          if(seq<packet->seq_num){
          n=fwrite(packet->buff,sizeof(char),MAXBUFSIZE,fp);
          //printf("write bytes: %d and seq_num %d \n",n,packet->seq_num);
          siz=siz-n;
          }

          if(seq==packet->seq_num){
            printf("drop \n");i++;
          }
          seq=packet->seq_num;

          pkt->seq=packet->seq_num;
          pkt->ack=packet->ack+1;
          sendto(sock,pkt,sizeof(ack_pk_t),0,(struct sockaddr *)&sin1, sizeof(remote));

  }
  free(packet);free(pkt);free(pkt1);
}

void send_file(char* file)
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
 	char ready[10]="ready";

	  if(sendto(sock, &siz,sizeof(siz),0, (struct sockaddr *)&sin1, sizeof(remote))<0)
				perror("send to:\n");
    if(sendto(sock, ready,sizeof(ready),0, (struct sockaddr *)&sin1, sizeof(remote))<0)
        perror("send to:\n");


  //packet pointer
	packet_t *packet=(packet_t*)malloc(sizeof(packet_t));
	ack_pk_t *pkt=(ack_pk_t*)malloc(sizeof(ack_pk_t));
	uint16_t num=1,ack=0;

	while(siz>0){

		n=fread(packet->buff,sizeof(char),MAXBUFSIZE,fp);
	  printf("Server reading Packet: %d \n",n);
		packet->seq_num=num;
		packet->ack=1;
		ack=1;
		while(ack<=1){
					nbytes=sendto(sock, packet,n+4,0,(struct sockaddr *)&sin1, sizeof(remote));
					printf("Server Sending Packet: %d \n",nbytes);
					//wait for ack
					printf("wait ack\n");

					nbytes=recvfrom(sock,pkt,sizeof(ack_pk_t), 0, (struct sockaddr *)&sin1, &remote_length);
					ack=pkt->ack;
					printf("ack%d\n",ack);
		}
		num++;
		siz=siz-n;
	}
	fclose(fp);
}

void send_file_server(){
  char file[20];
	if(recvfrom(sock, file,sizeof(file), 0, (struct sockaddr *)&sin1, &remote_length)<0)
			perror("recv:\n");
  printf("Filename Rec: %s\n",file);
  send_file(file);
}

void list_files()
{
  char file[]="ls_file";
  FILE *fp;
  fp=fopen(file,"w+");
	if(fp==NULL)
		perror("fopen:\n");
  system("ls -la> ls_file");

  send_file(file);

}


void delete_file(){

  //get file_name
  char file[20];
  char ack[10]="Success";
  int n;
  n=recvfrom(sock, file,20, 0, (struct sockaddr *)&sin1, &remote_length);
  printf("n %d\n",n);
  printf("%s\n",file);
  // use remove function to remove file_name
  if(remove(file)==0){
    sendto(sock,ack,sizeof(ack),0,(struct sockaddr *)&sin1, sizeof(remote));
    printf("Deleted Successfully\n");
  }
  else
      printf("Unable to delete file\n");


  // send ack of removing

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
	int option;

  while(1){
      recvfrom(sock, &option,sizeof(option), 0, (struct sockaddr *)&sin1,&remote_length);
      if(option==1){
     		// store file into server
     		put_in_server();
        option=0;
     	}
      if(option==2){
         send_file_server();
         option=0;
      }

      if(option==3){
         list_files();
         option =0;
      }
      if(option==4){
         delete_file();
         option =0;
      }
      if(option==5){
          //break;
          option =0;
      }
  }
 close(sock);
}
