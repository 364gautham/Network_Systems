#include "s_header.h"

struct sockaddr_in address;
int addrlen = sizeof(address);
int accept_fd,kill_server;
char s_conf[MAX];
char username[MAX],password[MAX];

void signal_handler(int sig){
  printf("Server Exiting!!\n");
  kill_server=1;
  close(accept_fd);
  exit(0);
}
bool is_dir(const char* path) {
    struct stat buf;
    stat(path, &buf);
    return S_ISDIR(buf.st_mode);
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
      address.sin_addr.s_addr = inet_addr("127.0.0.1");
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
int compute_filesize(char filename[MAX]){
	int filesize;
	fstream fd;
	fd.open(filename,fstream::in);
	if(fd.fail())	perror("open fail in file size compute function");
	fd.seekg(0,fstream::end);
	filesize=fd.tellg();
	fd.close();
	//cout<<"calculated File size is:"<<filesize<<endl;
	return filesize;
}
int read_dfs_file(){

  memset(username,0,MAX);
  memset(password,0,MAX);

  recv(accept_fd,username,MAX,0);
  recv(accept_fd,password,MAX,0);

  cout<<"Received Username and Password: "<<username<<password<<endl;

  fstream fd;
  int size=compute_filesize(s_conf);
  char buf[size],a[MAX],b[MAX];
  fd.open(s_conf,fstream::in|fstream::binary);
  fd.read(buf,size);
  fd.clear();
  fd.seekg (0, fstream::beg);
  char buff[MAXLINE];
  if(strstr(buf,username)){
    while(fd.getline(buff,MAXLINE)){
      if(strcmp(username,strtok(buff," "))==0){
          if(strcmp(password,strtok(NULL," \n"))==0){
            fd.close();
            return 1;
          }
      }
      memset(buff,0,MAXLINE);
    }
  }

  fd.close();
  return 0;
}
void encrypt_decrypt_data(char *buf,int size,char pwd[MAX]){
    int key=strlen(pwd);
    //cout<<"Encrypting the File\n";
    for(int i = 0; i <size; i++)
        buf[i] ^= key;
}
void receive_file(int server_num){
  char filename[MAX],subfolder[MAX];
  int filesize;
  //receive filesize
  recv(accept_fd,&filesize,sizeof(filesize),0);
  cout<<"Received filesizeis: "<<filesize<<endl;
  //receive filename
  memset(filename,0,sizeof(filename));
  recv(accept_fd,filename,sizeof(filename),0);
  cout<<"Received filename is: "<<filename<<endl;
  //receive subfolder
  memset(subfolder,0,sizeof(subfolder));
  recv(accept_fd,subfolder,sizeof(subfolder),0);
  cout<<"Received subfolder is : "<<subfolder<<endl;

  //receive filecontents
  //create subfolder
  char p_dir[200];
  memset(p_dir,0,200);
  getcwd(p_dir,200);

  // create server directory
  sprintf(p_dir+(strlen(p_dir)),"/DFS%d",server_num);
  cout<<"Server Directory is "<<p_dir<<endl;
  if(!(is_dir(p_dir))){
    if((mkdir(p_dir,0777))<0)
      perror("mkdir");
    else
      cout<<"Created Server Directory\n";
  }
  // create username folder
  sprintf(p_dir+(strlen(p_dir)),"/%s",username);
  cout<<"Username Directory is "<<p_dir<<endl;
  if(!(is_dir(p_dir))){
    if((mkdir(p_dir,0777))<0)
      perror("mkdir");
    else
      cout<<"Created username Directory\n";
  }

  //create subfolder for the username
  sprintf(p_dir+(strlen(p_dir)),"/%s",subfolder);
  cout<<"sub_folder Directory is "<<p_dir<<endl;
  if(!(is_dir(p_dir))){
    if((mkdir(p_dir,0777))<0)
      perror("mkdir");
    else
      cout<<"subfolder username Directory\n";
  }

  char create_file[200];
  sprintf(create_file,"%s%s",p_dir,filename);
  cout<<"File with Path is :"<<create_file<<endl;

  fstream fd;
  fd.open(create_file,fstream::out|fstream::binary);
  if(fd.fail())perror("File create");

  char buf[filesize];
  //while(filesize--){
    recv(accept_fd,buf,filesize,0);
    encrypt_decrypt_data(buf,filesize,password);
    fd.write(buf,filesize);
  //}
  fd.close();
  cout<<"Writing file is done\n";
}
void client_handle(int port){
    int option,s_port;
    recv(accept_fd,&option,sizeof(option),0);
    cout<<"Option: "<<option<<endl;
    switch(option){
      case PUT:
            int valid=1;
            // checking authenticity of user
            if(!read_dfs_file()){
              valid=0;
              send(accept_fd,&valid,sizeof(valid),0);
              break;
            }
            else{
              send(accept_fd,&valid,sizeof(valid),0);
              s_port=port%5; // 1,2,3,4 for DFS1,DFS2,DFS3,DFS4
              //first piece
              receive_file(s_port);
              //second piece
              receive_file(s_port);
              break;
            }
    }
}

int main(int argc ,char* argv[]){
      if (argc < 3){
        cout<<"USAGE: <port> <dfs.conf>\n";
        exit(1);
      }
      int port=atoi(argv[1]);
      int server_fd=create_socket(port);
      strcpy(s_conf,argv[2]);
      signal(SIGINT,signal_handler);
      kill_server=0;

      struct sockaddr_in client_addr;
      socklen_t client_len=sizeof(client_addr);

      while(!kill_server){
          cout<<"Waiting for Connection\n";
          accept_fd=accept(server_fd, (struct sockaddr *) &client_addr,&client_len);
          if(accept_fd==-1) perror("Accept:");
          if(fork () == 0 ){
              close(server_fd);
              client_handle(port);
              cout<<"\nDone handling the client\n"<<endl;
      			  exit(0);
          }
        close(accept_fd);
      }
      close(server_fd);
      return 0;
}
