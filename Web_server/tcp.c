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
int clients[CONNMAX];
char root[50]="/home/gautham/Network Systems/p_2/www";


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

void connection_handler(int slot){
   char header[10000],data[1024];
   int cont_len, n=0,size,fd, rcvd;;
   char a[20],str[20];
   char buf[1000],path[1000];
   char buffer[10240] = {0};

   char * token;
   char *file=malloc(40);
   char *version=malloc(10);
   char *method=malloc(10);
   char *cont_s = malloc(50);
   char *cont_ty = malloc(50);

  memset((void*)buffer,10240,0);
  rcvd=recv(clients[slot],buffer,1024,0);

  if(rcvd<0)
      printf("Receive error from client\n");
  else if(rcvd==0)
      printf("Client Disconnected unexpectedly\n");
  else{
              //printf("%s\n",buffer );

              //strcpy(buf,buffer);
              method = strtok (buffer, " \t\n");
              if ( strncmp(method, "GET\0", 4)==0 ){

                       file = strtok (NULL, " \t");
                       version = strtok (NULL, " \t\r\n");
                       //printf("version %s",version);
                      if ( strncmp( version, "HTTP/1.0", 8)!=0 && strncmp( version, "HTTP/1.1", 8)!=0 )
                      {
                        printf("bad request\n");
                        write(clients[slot], "HTTP/1.0 400 Bad Request\n", 25);
                      }
                      else
                      {
                        if ( strncmp(file, "/\0", 2)==0 )
                          file = "/index.html";        //Because if no file is specified, index.html will be opened by default

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
                          strcat(header,"\r\n\r\n");
                          //printf("%s",header);
                          if(send(clients[slot],header,strlen(header),0) == -1) {
                              printf("failed to send\n");
                          }

                          while ( (n=read(fd, data, BYTES))>0 )
                            write (clients[slot], data, n);
                        }
                        else   {
                          printf("file not found \n");
                          write(clients[slot], "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
                        }
                      }
                      // alarm for keepalive
              }

  }
  shutdown(clients[slot],SHUT_RDWR);
  close(clients[slot]);
  clients[slot]=-1;
}

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int slot=0;
    int addrlen = sizeof(address);

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

    while(1){
        if ((clients[slot] = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0){
            perror("accept");
            //exit(1);
        }
        else {
          if(fork()==0){
            connection_handler(slot);
            exit(0);
          }
        }

        while (clients[slot]!=-1) slot = (slot+1)%CONNMAX;

    }
    close(server_fd);
    return 0;
}
