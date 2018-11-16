#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
using namespace std;
extern int h_errno;

#define CONNMAX 1000
#define BYTES 1024
int clients[CONNMAX];
#define MAXBUFSIZE 10000

struct sockaddr_in address;
int addrlen = sizeof(address);

int create_socket(int port){
      int server_fd;
      // Creating socket file descriptor
      if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)   {
          perror("socket failed");
          exit(1);
      }
      int opt=1;
      if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt))){
          perror("setsockopt");
          exit(1);
      }
      address.sin_family = AF_INET;
      address.sin_addr.s_addr = INADDR_ANY;
      address.sin_port = htons(port);
      if (bind(server_fd, (struct sockaddr *)&address,sizeof(address))<0){
          perror("bind failed");
          exit(1);
      }
      if (listen(server_fd,100) < 0){
          perror("listen");
          exit(1);
      }
      return server_fd;
}
void client(int fd,char* host, char* buffer,char* port,char* path,char* http_version){

      int sockfd;
      struct addrinfo hints, *servinfo, *p;
      int rv;

      memset(&hints, 0, sizeof(hints));
      hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
      hints.ai_socktype = SOCK_STREAM;
      if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
          fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
          exit(1);
      }
      // loop through all the results and connect to the first we can
      for(p = servinfo; p != NULL; p = p->ai_next){
            if ((sockfd = socket(p->ai_family, p->ai_socktype,
                    p->ai_protocol)) == -1) {
                perror("socket");
                continue;
            }
            if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
                perror("connect");
                close(sockfd);
                continue;
            }
            break; // if we get here, we must have connected successfully
      }
      if (p == NULL) {
          // looped off the end of the list with no connection
          fprintf(stderr, "failed to connect\n");
          exit(2);
      }
      // create http request
      memset((void*)buffer,MAXBUFSIZE,0);
      sprintf(buffer,"GET https://%s %s %s\r\nConnection: close\r\n\r\n ",path,http_version,host);
      cout<<"\n"<<buffer<<endl;
      if(send(sockfd,buffer,strlen(buffer),0) == -1)
          cout<<"failed to send\n";

          int recv_bytes;
          char *recv_buffer= new char[MAXBUFSIZE];
          do{
            memset((void*)recv_buffer,MAXBUFSIZE,0);
            recv_bytes=recv(sockfd,recv_buffer,MAXBUFSIZE,0);
            if(recv_bytes)
                cout<<"\nServer Response:\n"<<recv_buffer<< "\n";
            if(send(fd,recv_buffer,recv_bytes,0) == -1)
                cout<<"failed to send\n";
          }while(recv_bytes>0);

      cout<<"\n end \n";
      shutdown(sockfd,SHUT_RDWR);
      close(sockfd);
      freeaddrinfo(servinfo); // all done with this structure
}

char* get_hostname(char* str){
    str=str+7;
    char *temp = strrchr(str,'/');
    //temp=temp+3;
    static char hostname[100];
    int i=0;
    while(*str!='/'){
      hostname[i++]=*str;
      str++;
    }
    //hostname[i++]=*str;
    hostname[i]='\0';
    return hostname;
}
char* get_port(char* str){
    str=str+7;
    int i=0;
    static char port[10];
    char* temp= strrchr(str,':');
    if(!temp){
      port[0]='8';port[1]='0';
      port[2]='\0';
      return port;
    }
    else{
      temp++;
      while(*temp!='\0'){
        port[i++]=*temp++;
      }
      port[i]='\0';
      return port;
    }
}
char* get_path(char* str){
    str=str+7;
    static char path[200];
    int i=0;
    char* temp= strrchr(str,':');
    if(!temp){
      strcpy(path,str);
      return path;
    }
    else{
      strncpy(path,str,strlen(str)-1);
      return path;
    }

}

void request_parse(int fd){
      int recv_bytes;
      char* buffer=new char[MAXBUFSIZE];
      //memset((void*)recv_buffer,10000,0);
      recv_bytes=recv(clients[fd],buffer,MAXBUFSIZE,0);
      if(recv_bytes)
          cout<<"\n"<<buffer<< "\n";

      hostent* valid_name=new hostent;

      char temp[3000];
      memcpy(temp,buffer,strlen(buffer));
      char* str=new char[100];
      str=strtok(temp," ");
      if(!strncmp(str,"GET",3)){
            //extract hostname,port,path
            str = strtok(NULL," ");

            char* hostname=get_hostname(str);
            cout<<hostname<<endl;
            char* port=get_port(str);
            cout<<port<<endl;
            char* path=get_path(str);
            cout<<path<<endl;
            valid_name=gethostbyname(hostname);
            str=strtok(NULL," ");
            if(!valid_name){
              cout<<"Host name failed : "<< h_errno<<endl;
              shutdown(clients[fd],SHUT_RDWR);
              close(clients[fd]);
              clients[fd]=-1;
              exit(1);
            }
            else{
              //cout<<(valid_name->h_name)<<endl;
              client(clients[fd],hostname,buffer,port,path,str);
            }
      }//end of get-check

      shutdown(clients[fd],SHUT_RDWR);
      close(clients[fd]);
      clients[fd]=-1;
      exit(1);

    //free(buffer);

}

int main(int argc ,char* argv[]){
      if (argc < 2){
        cout<<"USAGE: <server_port>\n";
        exit(1);
      }
      int i;
      for (i=0; i<CONNMAX; i++)
      clients[i]=-1;
      int port=atoi(argv[1]);
      int server_fd=create_socket(port);
      cout<<"First : "<<server_fd<<endl;
      int accept_fd=0;
      while(1){
            if ((clients[accept_fd] = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0){
                    perror("accept");
                    //exit(1);
            }
            else {
                  //printf("Socket descriptor: %d\n",clients[slot]);
                  if(fork()==0){
                    request_parse(accept_fd);
                    exit(0);
                  }
                  else{
                    //waitpid(-1,NULL,WNOHANG);
                    while (clients[accept_fd]!=-1) accept_fd = (accept_fd+1)%CONNMAX;
                  }
            }
      }
      close(server_fd);
      return 0;
}
