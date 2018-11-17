
#include "headers.h"

using namespace std;
extern int h_errno;

int timeout;
int clients[CONNMAX];
struct sockaddr_in address;
int addrlen = sizeof(address);

char* md5sum_create(const char* path){
      unsigned char digest[16];
      printf("string length: %lu\n", strlen(path));

      MD5_CTX ctx;
      MD5_Init(&ctx);
      MD5_Update(&ctx, path, strlen(path));
      MD5_Final(digest, &ctx);

      char* mdString=new char[33];
      for (int i = 0; i < 16; i++)
          sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);

      //printf("md5 digest: %s\n", mdString);
      return mdString;
}
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
      sprintf(buffer,"GET https://%s %s\r\nHost: %s\r\nConnection: close\r\n\r\n ",path,http_version,host);
      cout<<"\nProxy Request to Server:\n"<<buffer<<endl;
      if(send(sockfd,buffer,strlen(buffer),0) == -1)
          cout<<"failed to send\n";

      int recv_bytes;
      char *recv_buffer= new char[MAXBUFSIZE];
      char *filename;
      fstream file;
      filename=create_file(path);
      file.open(filename,fstream::out | fstream::binary);
      if(file.is_open()) cout<<"Proceed with storing cache\n";
      else cout<<"Error in creating Cache: fileopen\n";
      do{
        memset((void*)recv_buffer,MAXBUFSIZE,0);
        recv_bytes=recv(sockfd,recv_buffer,MAXBUFSIZE,0);
        if(recv_bytes)
            cout<<"\nServer Response:\n"<<recv_buffer<< "\n";
        if(send(fd,recv_buffer,recv_bytes,0) == -1)
            cout<<"failed to send\n";
        //write this data to cache
        file.write(recv_buffer,recv_bytes);
      }while(recv_bytes>0);
      file.close();
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
void handle_bad_hostname(int fd){
    char * err = new char[500];
   strcpy(err,"<HEAD><TITLE>HTTP 404 Not Found error message</TITLE></HEAD>\n");
   strcat(err,"<html><BODY>HTTP 404 Not Found error message: Bad Hostname");
   strcat(err,"\r\n");
   strcat(err,"</BODY></html>");
   if(send(fd,err,strlen(err),0) == -1) {
       printf("failed to send\n");
   }
}
void handle_non_GET(int fd){
   char * err = new char[500];
   strcpy(err,"<HEAD><TITLE>HTTP 400 Bad Request error message</TITLE></HEAD>\n");
   strcat(err,"<html><BODY>HTTP 400 Bad Request error message: Non_GET Request");
   strcat(err,"\r\n");
   strcat(err,"</BODY></html>");
   if(send(fd,err,strlen(err),0) == -1) {
       printf("failed to send\n");
   }
}
int search_f_timeout(char* hash){
    //first check: file existence
    if( access(hash,F_OK) != -1 ) {
      // file exists ,  check timeout
        cout<< "File Exists\n";
        //check for timeout
        char hash_filename[FILENAME_MAX];
        getcwd(hash_filename, FILENAME_MAX );
        strcat(hash_filename,"/");
        strcat(hash_filename,hash);
        struct stat attr;
        stat(hash_filename, &attr);
        char *time_file=new char[100];
        sprintf(time_file,"%s",ctime(&attr.st_mtime));
        char* temp = strtok(time_file," ");
        temp=strtok(NULL," ");
        temp=strtok(NULL," ");
        temp=strtok(NULL," ");
        char* hr= strtok(temp,":");
        char* min=strtok(NULL,":");
        char* sec=strtok(NULL,":");
        int t1=atoi(hr)*3600+atoi(min)*60+atoi(sec);
        bzero(time_file,sizeof(time_file));
        // get current time
        time_t rawtime;
        struct tm* timeinfo;
        time(&rawtime);
        timeinfo = localtime (&rawtime);
        time_file=asctime(timeinfo);
        temp = strtok(time_file," ");
        temp=strtok(NULL," ");
        temp=strtok(NULL," ");
        temp=strtok(NULL," ");
        hr= strtok(temp,":");
        min=strtok(NULL,":");
        sec=strtok(NULL,":");
        int t2=atoi(hr)*3600+atoi(min)*60+atoi(sec);
        //delete[] time_file;
        int diff_t=t2-t1;
        if(diff_t>60)return 0;
        else return 1;
    }
    else{
        // file doesn't exist
        return 0;
    }
}
int get_filesize(char* filename){
    // struct stat stat_buf;
    // int rc = stat(filename, &stat_buf);
    // return rc == 0 ? stat_buf.st_size : -1;
    return 1;
}
int get_filecreation_time(char* filename){

}
char* create_file(char* path){
    fstream cache;
    //create hash_key
    char *hash_key=new char[200];
    hash_key=md5sum_create(path);

    cache.open(hash_key);
    if(cache.is_open())cout<<"File creation Success\n";
    else cout<<"Error: create_file()\n";
    cache.close();

    return hash_key;
}
int check_cache(char* path){

    char *hash_key=new char[200];
    hash_key=md5sum_create(path);
    //cout << "Hash"<<hash_key;
    if(search_f_timeout(hash_key)){
      //to send from cache
      //delete[] hash_key;
      return 1;
    }
    else{
      //store in cache
        //delete[] hash_key;
        return 0;
    }
}
void send_from_cache(int fd,char* path){
    char *hash_key=new char[200];
    hash_key=md5sum_create(path);
    streamsize data_size;
    //get file_size
    fstream cache;
    int size,recv_bytes;
    char *recv_buffer=new char[MAXBUFSIZE];
    if((size = get_filesize(hash_key)) == -1){
        cout<<"Error in file : Filesize \n";
    }
    else{
      //read from file
      cache.open(hash_key,fstream::in | fstream::binary);
      if (cache.is_open())
      {
          while (!cache.eof()){
              cache.read(recv_buffer,MAXBUFSIZE);
              data_size=cache.gcount();
              if(send(fd,recv_buffer,data_size,0) == -1)
                cout<<"failed to send\n";
          }
          cache.close();
      }

    }
    //delete[] hash_key;
    //delete[] recv_buffer;



}
void request_parse(int fd){
      int recv_bytes;
      char* buffer=new char[MAXBUFSIZE];
      fstream link;
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
            cout<<"Hostname:"<<hostname<<endl;
            char* port=get_port(str);
            cout<<"Port:"<<port<<endl;
            char* path=get_path(str);
            cout<<"Path:"<<path<<endl;
            valid_name=gethostbyname(hostname);
            str=strtok(NULL," \r\n");
            if(!valid_name){
              cout<<"Host name failed : "<< h_errno<<endl;
              handle_bad_hostname(clients[fd]);
              shutdown(clients[fd],SHUT_RDWR);
              close(clients[fd]);
              clients[fd]=-1;
              exit(1);
            }
            else{
              //cout<<(valid_name->h_name)<<endl;
                  if(check_cache(path)){
                    //send from cache
                    cout<<"Sending From Cache\n";
                    send_from_cache(clients[fd],path);
                  }
                  else{
                    //get from server and store in cache
                    cout<<"FILE not found in cache, Request from server\n";
                    client(clients[fd],hostname,buffer,port,path,str);
                  }

            }
      }//end of get-check
      else{
        handle_non_GET(clients[fd]);
      }

      shutdown(clients[fd],SHUT_RDWR);
      close(clients[fd]);
      clients[fd]=-1;
      exit(1);


}

int main(int argc ,char* argv[]){
      if (argc < 3){
        cout<<"USAGE: <server_port> <timeout for cache>\n";
        exit(1);
      }
      int i;
      for (i=0; i<CONNMAX; i++)
      clients[i]=-1;
      int port=atoi(argv[1]);
      timeout = atoi(argv[2]);
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