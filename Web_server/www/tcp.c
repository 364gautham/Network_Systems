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
#include <errno.h>
#include <sys/stat.h>
#include <pthread.h>


#define CONNMAX 1000
#define BYTES 1024
int kill_server,server_fd;
int clients[CONNMAX];
int slot,conn_aliv;

char root[50]="/home/gautham/Network Systems/p_2/www";

void s_exit(){
  kill_server=1;
  close(server_fd);
  exit(0);
}
void alive_exit(){
  printf("Timeout\n");
  shutdown(clients[slot],SHUT_RDWR);
  close(clients[slot]);
  clients[slot]=-1;
  conn_aliv=0;
  exit(0);

}
void reverse(char s[])
 {
     int i, j;char c;
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];s[i] = s[j];s[j] = c;
     }
}
void file_size(char *file,char *str){
      FILE *fp;
      //char file_n[100]="/"
      fp=fopen(file,"r");
      if(fp==NULL){
        perror("file open");
        return ;
      }
      fseek(fp, 0, SEEK_END);
      int size = ftell(fp);
      fseek(fp, 0, SEEK_SET);
      //printf("\n size %d",size);
      int i=0;
      do{
         str[i++] = size % 10 + '0';
      }while ((size /= 10) > 0);
     str[i] = '\0';
     reverse(str);
}

char* content_type(char *file_n){
  char *ext = strrchr(file_n, '.');
  ext++;
  //strlower(ext);
  if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0) { return "text/html"; }
  if (strcmp(ext, "jpeg") == 0 || strcmp(ext, "jpg") == 0) { return "image/jpg"; }
  if (strcmp(ext, "css") == 0) { return "text/css"; }
  if (strcmp(ext, "js") == 0) { return "application/javascript"; }
  if (strcmp(ext, "json") == 0) { return "application/json"; }
  if (strcmp(ext, "txt") == 0) { return "text/plain"; }
  if (strcmp(ext, "gif") == 0) { return "image/gif"; }
  if (strcmp(ext, "png") == 0) { return "image/png"; }
  if (strcmp(ext, "ico") == 0) { return "image/x-icon"; }
}

void bad_version(char *header,char* version,int slot){
  strcpy(header,version);
  strcat(header," 500 Internal Server Error\r\n");
  strcat(header,"Content-Type: Invalid");
  strcat(header,"\r\n");
  strcat(header,"Content-Length: Invalid");
  strcat(header,"\r\n\r\n");
  printf("%s",header);
  if(send(clients[slot],header,strlen(header),0) == -1) {
      printf("failed to send\n");
  }
 char * err = malloc(500);
  strcpy(err,"<HEAD><TITLE>500 Internal Server Error</TITLE></HEAD>\n");
  strcat(err,"<html><BODY>500 Internal Server Error: Invalid HTTP Version:");
  strcat(err,version);
  strcat(err,"\r\n");
  strcat(err,"</BODY></html>");
  if(send(clients[slot],err,strlen(err),0) == -1) {
      printf("failed to send\n");
  }

}

void bad_file(char *header,char* version,int slot){
  strcpy(header,version);
  strcat(header," 500 Internal Server Error\r\n");
  strcat(header,"Content-Type: Invalid");
  strcat(header,"\r\n");
  strcat(header,"Content-Length: Invalid");
  strcat(header,"\r\n\r\n");
  printf("%s",header);
  if(send(clients[slot],header,strlen(header),0) == -1) {
      printf("failed to send\n");
  }
 char * err = malloc(500);
  strcpy(err,"<HEAD><TITLE>500 Internal Server Error</TITLE></HEAD>\n");
  strcat(err,"<html><BODY>500 Internal Server Error: File not found:");
  strcat(err,"\r\n");
  strcat(err,"</BODY></html>");
  if(send(clients[slot],err,strlen(err),0) == -1) {
      printf("failed to send\n");
  }

}

void connection_handler(int slot){
   char data[1024];
   int cont_len, n=0,size,fd, rcvd;
   char a[20],str[20];
   char buf[1000],path[1000];
   char buffer[10240] = {0};
   char *header = malloc(10000);
   char * token;
   char *file=malloc(40);
   char *version=malloc(10);
   char *method=malloc(10);
   char *cont_s = malloc(50);
   char *cont_ty = malloc(50);
   char *alive = malloc(30);
   char *error =malloc(2000);
   char *post =malloc(2000);
   char buf_p[500];
   conn_aliv=1;
   int exit_al =1;
while(conn_aliv){
  conn_aliv=0;exit_al=1;
  memset((void*)buffer,10240,0);
  memset((void*)buf,1000,0);
  memset((void*)header,10000,0);



  rcvd=recv(clients[slot],buffer,10240,0);

  if(rcvd<0)
      printf("Receive error from client\n");
  else if(rcvd==0)
      printf("Client Disconnected unexpectedly\n");
  else{
              printf("%s\n",buffer );
              strcpy(buf,buffer);

              //strcpy(buf,buffer);
              method = strtok (buffer, " \t\n");
              if ( strncmp(method, "GET\0", 4)==0 ){

                      file = strtok (NULL, " \t");
                      version = strtok (NULL, " \t\r\n");
                       //printf("version %s",version);
                      if ( strncmp( version, "HTTP/1.0", 8)!=0 && strncmp( version, "HTTP/1.1", 8)!=0 )
                      {
                        printf("bad request version \n");exit_al=0;
                        bad_version(header,version,slot);
                      }
                      else
                      {

                            if ( strncmp(file, "/\0", 2)==0 )
                              file = "/index.html";        //Because if no file is specified, index.html will be opened by default
                            printf("%s\n",file);
                            strcpy(path, root);
                            strcpy(&path[strlen(root)], file);
                            //printf("file: %s\n", path);
                            cont_ty=content_type(path);
                            if ( (fd=open(path, O_RDONLY))!=-1 )    //FILE FOUND
                            {
                                    file_size(path,str);
                                    strcpy(header,version);
                                    strcat(header," 200 OK\r\n");
                                    strcat(header,"Content-Type: ");
                                    strcat(header,cont_ty);
                                    strcat(header,"\r\n");
                                    strcat(header,"Content-Length: ");
                                    strcat(header,str);
                                    if(strstr(buf,"keep-alive")!=NULL){
                                      strcat(header,"\r\n");
                                      strcat(header,"Connection: Keep-alive");
                                    }
                                    else{
                                      strcat(header,"\r\n");exit_al=0;
                                      strcat(header,"Connection: close");
                                    }
                                    strcat(header,"\r\n\r\n");
                                    printf("%s",header);
                                    if(send(clients[slot],header,strlen(header),0) == -1) {
                                        printf("failed to send\n");exit_al=0;
                                    }

                                    while ( (n=read(fd, data, BYTES))>0 )
                                      write (clients[slot], data, n);
                             }

                             else{
                              printf("file not found \n");exit_al=0;
                              bad_file(header,version,slot);
                            }
                      }
              }
              else if (strncmp(method, "POST\0", 5)==0){

                file = strtok (NULL, " \t");
                version = strtok (NULL, " \t\r\n");
                //printf("version %s",version);
                if ( strncmp( version, "HTTP/1.0", 8)!=0 && strncmp( version, "HTTP/1.1", 8)!=0 )
                {
                  printf("bad request version \n");exit_al=0;
                  bad_version(header,version,slot);
                }
                else
                {
                      if ( strncmp(file, "/\0", 2)==0 )
                        file = "/index.html";        //Because if no file is specified, index.html will be opened by default
                      //printf("%s\n",file);
                      strcpy(path, root);
                      strcpy(&path[strlen(root)], file);
                      //printf("file: %s\n", path);
                      cont_ty=content_type(path);
                      if ( (fd=open(path, O_RDONLY))!=-1 )    //FILE FOUND
                      {
                              file_size(path,str);
                              strcpy(header,version);
                              strcat(header," 200 OK\r\n");
                              strcat(header,"Content-Type: ");
                              strcat(header,cont_ty);
                              strcat(header,"\r\n");
                              strcat(header,"Content-Length: ");
                              strcat(header,str);
                              if(strstr(buf,"keep-alive")!=NULL){
                                strcat(header,"\r\n");
                                strcat(header,"Connection: Keep-alive");
                              }
                              else{
                                strcat(header,"\r\n");exit_al=0;
                                strcat(header,"Connection: close");
                              }
                              //printf("%s",header);
                              strcat(header,"\r\n");
                              char *pt_post=strstr(buf,"POSTDATA");
                              strcpy(buf_p,pt_post);
                              if(strstr(buf_p,"\n")!=NULL){
                                strtok (buf_p, " \r\n");
                                post = strtok (NULL, " \r\n");
                                strcat(header,"<html><body><pre><h1>POST DATA</h1>");
                                //strcpy(post,pt_post);
                                //printf("%s",post);
                                strcat(header,post);strcat(header,"</pre>");

                              }
                              else{
                                strcat(header,"<html><body><pre><h1>POST DATA</h1>");
                                strcat(header,"</pre>");
                              }

                              //strcat(header,"\r\n\r\n");
                              printf("%s",header);
                              if(send(clients[slot],header,strlen(header),0) == -1) {
                                  printf("failed to send\n");exit_al=0;
                              }

                              while ( (n=read(fd, data, BYTES))>0)
                                write (clients[slot], data, n);
                            //printf("");
                       }

                       else{
                        printf("file not found \n");exit_al=0;
                        bad_file(header,version,slot);
                      }
                }



              }

              else{
                printf("bad request\n");
                bad_version(header,version,slot);
              }

              if(strstr(buf,"keep-alive")!=NULL){
                printf("\nTimer Started for 10 seconds\n");
                //if(exit_al)
                conn_aliv=1;
                alarm(10);
              }
   }

  }
  shutdown(clients[slot],SHUT_RDWR);
  close(clients[slot]);
  clients[slot]=-1;
}

int main(int argc, char const *argv[])
{
    int new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;

    int addrlen = sizeof(address);
    signal(SIGINT, s_exit);
    signal(SIGALRM, alive_exit);
    slot=0;
    if (argc < 2){
		    printf("USAGE: <server_port>\n");
		    exit(1);
	  }
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)   {
        perror("socket failed");
        exit(1);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt))){
        perror("setsockopt");
        exit(1);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(atoi(argv[1]));

    int i;
	  for (i=0; i<CONNMAX; i++)
		clients[i]=-1;

    if (bind(server_fd, (struct sockaddr *)&address,sizeof(address))<0){
        perror("bind failed");
        exit(1);
    }
    if (listen(server_fd,10000) < 0){
        perror("listen");
        exit(1);
    }
    kill_server=0;
    while(!kill_server){
        if ((clients[slot] = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0){
            perror("accept");
            //exit(1);
        }

        else {
          printf("Val %d\n",clients[slot]);
          if(fork()==0){
            connection_handler(slot);
            exit(1);
          }else{
            while (clients[slot]!=-1) slot = (slot+1)%CONNMAX;
          }
        }

    }
    close(server_fd);
    return 0;
}
