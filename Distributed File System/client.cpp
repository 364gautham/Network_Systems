#include "headers.h"


struct server_data{
	char ip_address[33];
	int port;
};
struct file_data{
  char piece_name[100];
  int piece_size;
};
file_data piece[NUM_SERVERS];
server_data server[NUM_SERVERS];

char username[MAX],password[MAX],filename[MAX],sub_folder[MAX];
int sockfd[NUM_SERVERS],kill_client;

void signal_handler(int sig){
  printf("Client Exiting!!\n");
  kill_client=1;
  for(int i=0;i<NUM_SERVERS;i++)
    close(sockfd[i]);
  exit(0);
}
void read_dfc_file(const char filename[50]){
  fstream dfc_file;
  char buff[2000],port_str[100];
  dfc_file.open(filename,fstream::in|fstream::binary);
  while(dfc_file.getline(buff,MAXLINE)){
  		if(strstr(buff,"Server DFS1")){
  			strtok(buff," ");
  			strtok(NULL," ");
  			strcpy(server[0].ip_address,strtok(NULL,":"));
  			strcpy(port_str,strtok(NULL,"NULL"));
  			server[0].port=atoi(port_str);
  			cout<<"server 1 IP address: "<<server[0].ip_address<<"server 1 port number: "<<server[0].port<<endl;
  		}
  		else if(strstr(buff,"Server DFS2")){
  			strtok(buff," ");
  			strtok(NULL," ");
  			strcpy(server[1].ip_address,strtok(NULL,":"));
  			strcpy(port_str,strtok(NULL,"NULL"));
  			server[1].port=atoi(port_str);
  			cout<<"server 2 IP address: "<<server[1].ip_address<<"server 2 port number: "<<server[1].port<<endl;
  		}
  		else if(strstr(buff,"Server DFS3")){
  			strtok(buff," ");
  			strtok(NULL," ");
  			strcpy(server[2].ip_address,strtok(NULL,":"));
  			strcpy(port_str,strtok(NULL,"NULL"));
  			server[2].port=atoi(port_str);
  			cout<<"server 3 IP address: "<<server[2].ip_address<<"server 3 port number: "<<server[2].port<<endl;
  		}
  		else if(strstr(buff,"Server DFS4")){
  			strtok(buff," ");
  			strtok(NULL," ");
  			strcpy(server[3].ip_address,strtok(NULL,":"));
  			strcpy(port_str,strtok(NULL,"NULL"));
  			server[3].port=atoi(port_str);
  			cout<<"server 4 IP address: "<<server[3].ip_address<<"server 4 port number: "<<server[3].port<<endl;
  		}
  		else if(strstr(buff,"User")){
  			strtok(buff," ");
  			strcpy(username,strtok(NULL,"NULL"));
  			cout<<"UserName: "<<username<<endl;
  		}
  		else if(strstr(buff,"Password")){
        strtok(buff," ");
			  strcpy(password,strtok(NULL,"NULL"));
			  cout<<"Password: "<<password<<endl;
		  }
  }
  dfc_file.close();
}
void create_client_connections(){
    struct sockaddr_in serv_addr;
    int timeout=1;
    for(int i=0;i<NUM_SERVERS;i++){
        if ((sockfd[i] = socket(AF_INET, SOCK_STREAM, 0)) == 0)   {
          perror("socket failed");
          exit(1);
        }
        serv_addr.sin_family = AF_INET; // use AF_INET6 to force IPv6
        serv_addr.sin_port = htons(server[i].port);
        serv_addr.sin_addr.s_addr =inet_addr(server[i].ip_address);
        // timeout for sockets
        struct timeval time_out;
        time_out.tv_sec = timeout;
        time_out.tv_usec = 0;
        if (setsockopt (sockfd[i], SOL_SOCKET, SO_RCVTIMEO, (char *)&time_out,sizeof(time_out)) < 0){
            perror("setsockopt failed");
        }
        if(connect(sockfd[i],(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
          		cout<<"create_client_connections function\n"<<endl;
          		perror("connect:");
              exit(1);
        }
        memset(&serv_addr, 0, sizeof(serv_addr));
    }
}
int user_menu(){
    cout<<"\n\n***Enter Options as given below***\n";
    cout<<"1: put 'Filename' 'subfolder/' \n2: get 'Filename' 'subfolder/'\n3: list 'subfolder/' \n\n";
    char input[100];
		char* temp;

    while(1){
      memset(filename,0,MAX);
      memset(sub_folder,0,MAX);
      memset(input,0,100);
      fgets(input,100,stdin);
      if(strncmp(input,"put",3)==0){
        strtok(input," ");
        strcpy(filename,strtok(NULL," "));
        cout<<"Filename: "<<filename<<endl;
				temp=strtok(NULL," \n");
				if(temp==NULL) return INVALID;
        strcpy(sub_folder,temp);
        cout<<"Folder entered: "<<sub_folder<<endl;
        return PUT;
      }
      if(strncmp(input,"get",3)==0){
        strtok(input," ");
        strcpy(filename,strtok(NULL," "));
        cout<<filename<<endl;
				temp=strtok(NULL," \n");
				if(temp==NULL) return INVALID;
        strcpy(sub_folder,temp);
        cout<<sub_folder<<endl;
        return GET;
      }
      if(strncmp(input,"list",4)==0){
          strtok(input," ");
					temp=strtok(NULL," \n");
					if(temp==NULL) return INVALID;
          strcpy(sub_folder,temp);
          cout<<sub_folder<<endl;
          return LIST;
      }
      else if(!(strstr(input,"/")))
          cout<<"Wrong command, Enter Again\n";
    }
    return 0;
}
int access_grant(int option){
  int success;
  for(int i=0;i<NUM_SERVERS;i++){
    success=0;
    //send option to servers
    send(sockfd[i],&option,sizeof(option),0);
    send(sockfd[i],username,sizeof(username),0);
    send(sockfd[i],password,sizeof(password),0);
    recv(sockfd[i],&success,sizeof(success),0);
		cout<<"Success:"<<success<<endl;
    if(!success){
      cout<<"Access Grant Issue Server: "<<i+1<<endl;
      return 0;
    }
  }
  return 1;
}
int compute_filesize(char filename[MAX]){
	int filesize;
	fstream fd;
	fd.open(filename,fstream::in);
	if(fd.fail())	perror("open fail in file size compute function");
	fd.seekg(0,fstream::end);
	filesize=fd.tellg();
	fd.close();
	cout<<"calculated File size is:"<<filesize<<endl;
	return filesize;
}
void split_files_in_4(char filename[MAX]){
  int i;
  int size=compute_filesize(filename);
  int split=size/4;
  int rem_size=size-(split*3);
  fstream f1,f2,f3,f4,fd;

  // create filenames accordingly
  for(i=0;i<NUM_SERVERS;i++){
    memset(piece[i].piece_name, 0, sizeof(piece[i].piece_name));
    memset(&piece[i].piece_size, 0, sizeof(piece[i].piece_size));
    sprintf(piece[i].piece_name,".%s.%d",filename,i+1);
    if(i==3)piece[i].piece_size=rem_size;
    else piece[i].piece_size=split;
  }
  f1.open(piece[0].piece_name,fstream::out|fstream::binary);
	if(f1.fail())	perror("f1 open");
  f2.open(piece[1].piece_name,fstream::out|fstream::binary);
  if(f2.fail())	perror("f2 open");
  f3.open(piece[2].piece_name,fstream::out|fstream::binary);
  if(f3.fail())	perror("f3 open");
  f4.open(piece[3].piece_name,fstream::out|fstream::binary);
  if(f4.fail())	perror("f4 open");

  fd.open(filename,fstream::in|fstream::binary);
  if(fd.fail())	perror("fd open");

  char buf[size];
  fd.read(buf,split);
  f1.write(buf,split);

  fd.read(buf,split);
  f2.write(buf,split);

  fd.read(buf,split);
  f3.write(buf,split);

  fd.read(buf,rem_size);
  f4.write(buf,rem_size);

  f1.close();f2.close();f3.close();f4.close();fd.close();

  // for(i=0;i<NUM_SERVERS;i++)
  //   cout<<piece[i].piece_name<<endl;
  // for(i=0;i<NUM_SERVERS;i++)
  //   cout<<piece[i].piece_size<<endl;
}
int md5hash_index(char filename[MAX]){
  unsigned char c[MD5_DIGEST_LENGTH], data[100];
  FILE* fd = fopen(filename, "rb");

  if (fd<= 0) {
    fprintf(stderr, "Unable to open file: %s", filename);
    return -1;
  }
  MD5_CTX md_context;
  int n_bytes, i, mod;

  MD5_Init(&md_context);
  memset(data, 0, sizeof(data));
  while ((n_bytes = fread(data,1,100, fd)) != 0)
      MD5_Update(&md_context, data, n_bytes);
  MD5_Final(c, &md_context);
  fclose(fd);
  mod=0;
  for (i = 0; i < MD5_DIGEST_LENGTH; i++){
      //printf("%02x", c[i]);
      mod = (mod * 16 + (unsigned int)c[i]) % NUM_SERVERS;
  }
  return mod;
}
void encrypt_decrypt_data(char *buf,int size,char pwd[MAX]){
    int key=strlen(pwd);
    //cout<<"Encrypting the File\n";
    for(int i = 0; i <size; i++)
        buf[i] ^= key;
}
void send_f(int server_num,int file_piece){
    //send_filesize
    //cout<<"sending Filesize "<<piece[file_piece].piece_size;
    send(sockfd[server_num],&piece[file_piece].piece_size,sizeof(piece[file_piece].piece_size),0);
    //send filename
    send(sockfd[server_num],piece[file_piece].piece_name,sizeof(piece[file_piece].piece_name),0);
    //send sub_folder
    send(sockfd[server_num],sub_folder,sizeof(sub_folder),0);
    // send file contents after encrypting
    fstream fd;
    fd.open(piece[file_piece].piece_name,fstream::in|fstream::binary);
    int size=piece[file_piece].piece_size;
    char* buff = new char[size];
    memset(buff, 0, sizeof(buff));
    fd.read(buff,size);
    encrypt_decrypt_data(buff,size,password);
    send(sockfd[server_num],buff,size,0);
    //cout<<"File Sent after Encryption to Server\n"<<endl;
    delete buff;
    fd.close();
}
void send_files(char filename[MAX]){
    //get hash index
    int mod=md5hash_index(filename);
    cout<<"Mod value is "<<mod<<endl;
    int a[4][4][2]={
      { { 1, 2 }, { 2, 3 }, { 3, 4 }, { 4, 1 } },
      { { 4, 1 }, { 1, 2 }, { 2, 3 }, { 3, 4 } },
      { { 3, 4 }, { 4, 1 }, { 1, 2 }, { 2, 3 } },
      { { 2, 3 }, { 3, 4 }, { 4, 1 }, { 1, 2 } }
    };
   int i,file_piece,j;
   //4 servers
   for(j=0;j<NUM_SERVERS;j++){
       //files per server
       for(i=0;i<2;i++){
         file_piece=a[mod][j][i];
         send_f(j,file_piece-1);
       }
   }
}

int main(int argc, char const *argv[]){

  if (argc < 2){
    cout<<"USAGE: <config file - dfc.conf>\n";
    exit(1);
  }
  signal(SIGINT,signal_handler);
  int choice;
  kill_client=0;
  // get servers info and user info
  read_dfc_file(argv[1]);
  // connect to servers
  create_client_connections();

  while(!kill_client){
        choice=user_menu();
        switch(choice){
          case PUT:
            // put the file into servers
            if(access_grant(PUT)){
              // divide the file into 4 parts
              split_files_in_4(filename);
              send_files(filename);
              cout<<"Sent files to server in PUT\n";
              break;
            }
            else{
              cout<<"Invalid Username/Password. Please try again.\n";
              break;
            }
					case INVALID:
							cout<<"Enter again\n";
							break;


        }
  }
	return 0;
}
