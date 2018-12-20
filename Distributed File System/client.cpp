/********************************************************************************************************
*
* UNIVERSITY OF COLORADO BOULDER
*
* @file client.cpp
* @brief DIstributed File System: Client Program: Stores files onto connected server and fecthes them.
*
* @author  Gautham K A
* @date  12/15/2018
*
********************************************************************************************************/
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
							sockfd[i]=0;
        }
        memset(&serv_addr, 0, sizeof(serv_addr));
    }
}
int user_menu(){
    cout<<"\n\n*** Enter Options as given below : Folder name shoudl end with '/' ***\n";
    cout<<"1: put 'Filename' 'subfolder/' \n2: get 'Filename' 'subfolder/'\n3: list 'subfolder/' \n\n";
    char input[100],file_n[MAX];
		char* temp;

    while(1){
      memset(filename,0,MAX);
      memset(sub_folder,0,MAX);
      memset(input,0,100);
			memset(file_n,0,100);
      fgets(input,100,stdin);
      if(strncmp(input,"put",3)==0){
        strtok(input," ");
				temp=strtok(NULL," ");
				if(temp==NULL)return 0;
        strcpy(file_n,temp);
        cout<<"Filename: "<<file_n<<endl;
				temp=strtok(NULL," \n");
				if(temp==NULL) {strncpy(filename,file_n,strlen(file_n)-1);return PUT;}
        strcpy(sub_folder,temp);
				if(!strstr(sub_folder,"/")) return 0;
				strcpy(filename,file_n);
        cout<<"Folder entered: "<<sub_folder<<endl;
        return PUT;
      }
      else if(strncmp(input,"get",3)==0){
        strtok(input," ");
				temp=strtok(NULL," ");
				if(temp==NULL)return 0;
        strcpy(file_n,temp);
				cout<<"Filename: "<<file_n<<endl;
				temp=strtok(NULL," \n");
				if(temp==NULL) {strncpy(filename,file_n,strlen(file_n)-1);return GET;}
        strcpy(sub_folder,temp);
				if(!strstr(sub_folder,"/")) return 0;
				strcpy(filename,file_n);
				cout<<"Folder entered: "<<sub_folder<<endl;
        return GET;
      }
      else if(strncmp(input,"list",4)==0){
          strtok(input," ");
					temp=strtok(NULL," \n");
					if(temp==NULL) return LIST;
          strcpy(sub_folder,temp);
					if(!strstr(sub_folder,"/")) return 0;
          cout<<sub_folder<<endl;
          return LIST;
      }
      else
          cout<<"Wrong command, Enter Again\n";
    }
}
int access_grant(int option){
  int success;
  for(int i=0;i<NUM_SERVERS;i++){
    	success=0;
    	//send option to servers
			if(sockfd[i]>0){
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
	//cout<<"calculated File size is:"<<filesize<<endl;
	return filesize;
}
int split_files_in_4(char filename[MAX]){
  int i;
	fstream f1,f2,f3,f4,fd;
	//if file not present
	fd.open(filename,fstream::in|fstream::binary);
  if(fd.fail()){
		perror("fd open");
		return 0;
	}
	fd.close();
  int size=compute_filesize(filename);
  int split=size/4;
  int rem_size=size-(split*3);


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
	return 1;
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
void send_helper(int server_num,int file_piece){
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
         send_helper(j,file_piece-1);
       }
   }
}
bool fileExists(const char *fileName){
    ifstream infile(fileName);
    return infile.good();
}
void combine_files(){
		fstream f1,f2,f3,f4,fd;
		char fn1[MAX],fn2[MAX],fn3[MAX],fn4[MAX];
		sprintf(fn1,".%s.1",filename);
		sprintf(fn2,".%s.2",filename);
		sprintf(fn3,".%s.3",filename);
		sprintf(fn4,".%s.4",filename);

		f1.open(fn1,fstream::in|fstream::binary);
		f2.open(fn2,fstream::in|fstream::binary);
		f3.open(fn3,fstream::in|fstream::binary);
		f4.open(fn4,fstream::in|fstream::binary);
		if(!(f1.fail() || f2.fail() || f3.fail() || f4.fail())){
				//get filesize and read -write
				fd.open(filename,fstream::out|fstream::binary);
				int f_size=compute_filesize(fn1);
				char *buff =new char[f_size];
				f1.read(buff,f_size);
				fd.write(buff,f_size);
				memset(buff,0,f_size);
				f2.read(buff,f_size);
				fd.write(buff,f_size);
				memset(buff,0,f_size);
				f3.read(buff,f_size);
				fd.write(buff,f_size);
				memset(buff,0,f_size);
				f_size=0;
				delete buff;
				//since last piece of different size
				f_size=compute_filesize(fn4);
				char *buf=new char[f_size];
				f4.read(buf,f_size);
				fd.write(buf,f_size);
				delete buf;
				cout<<"File is written into Project folder, GET is Successful\n";
		}
		else{
			cout<<"File Incomplete: Cannot get the file , Required Servers not Active\n";
		}
		fd.close();f1.close();f2.close();f3.close();f4.close();
}
void get_all_pieces(){
		// send filename and subfolder data
		int i,check[4],ack=0,serv_live[4];
		for(i=0;i<NUM_SERVERS;i++){
			send(sockfd[i],filename,sizeof(filename),0);
			//send sub_folder
			send(sockfd[i],sub_folder,sizeof(sub_folder),0);
		}
		for(i=0;i<NUM_SERVERS;i++){
				if(recv(sockfd[i],&ack,sizeof(int),0)<=0)
					cout<<"Receive: Server Not Alive:"<<i+1<<endl;
				if(ack)serv_live[i]=1;
				else serv_live[i]=0;
				ack=0;
				cout<<"SERV_LIVE: "<<serv_live[i]<<endl;
		}
		int filesize=0,n;
		fstream fd;
		char file_n[MAX];
		//server 1
		for(i=0;i<NUM_SERVERS;i++){
			if(serv_live[i]){
					memset(file_n,0,100);
					recv(sockfd[i],&filesize,sizeof(int),0);
					cout<<"Filesize:"<<filesize<<endl;

					char *buf1=new char[filesize];
					recv(sockfd[i],file_n,sizeof(file_n),0);
					n=recv(sockfd[i],buf1,filesize,0);
					cout<<"recive bytes:"<<n<<endl;

					cout<<"file open:"<<file_n<<endl;
					fd.open(file_n,fstream::out|fstream::binary);
					if(fd.fail())perror("fopen1");
					encrypt_decrypt_data(buf1,filesize,password);
					fd.write(buf1,filesize);
					fd.close();
					delete buf1;

					//piece 2
					memset(file_n,0,100);
					filesize=0;
					recv(sockfd[i],&filesize,sizeof(int),0);
					cout<<"Filesize:"<<filesize<<endl;

					char *buf2=new char[filesize];
					recv(sockfd[i],file_n,sizeof(file_n),0);
					n=recv(sockfd[i],buf2,filesize,0);
					cout<<"recive bytes:"<<n<<endl;
					cout<<"file open:"<<file_n<<endl;

					fd.open(file_n,fstream::out|fstream::binary);
					if(fd.fail())perror("fopen2");
					encrypt_decrypt_data(buf2,filesize,password);
					fd.write(buf2,filesize);
					fd.close();
					filesize=0;
					delete buf2;
			}
		}

		combine_files();

}

void read_listfile(char* data, char fname[MAX],int fs){
		//char* complete_list=new char[fs];
		fstream fd;
		char *buff=new char[MAX];
		fd.open(fname,fstream::in|fstream::binary);
		if(fd.fail())perror("list file open");
		char *name=new char[MAX];
		char *ncheck=new char[MAX];
		char *complete_list=new char[1024];
		int flag=0;
		char* temp;
		memset(buff,0,MAX);memset(name,0,MAX);memset(complete_list,0,MAX);
		//cout<<data<<endl;
		while(fd.getline(buff,MAX)){
			if(!(strcmp(buff,".")==0||strcmp(buff,"..")==0)){
				//extract filename
				temp=buff+1;
				strncpy(name,temp,strlen(buff)-3);
				//cout<<name<<endl;
				//check for all pieces in buffer
				sprintf(ncheck,".%s.1",name);
				if(strstr(data,ncheck)){
					//check piece2
					strncpy(ncheck+strlen(ncheck)-1,"2",1);
					{
						if(strstr(data,ncheck)){
							//check piece 3
							strncpy(ncheck+strlen(ncheck)-1,"3",1);
							if(strstr(data,ncheck)){
								//check piece 4
								strncpy(ncheck+strlen(ncheck)-1,"4",1);
								if(strstr(data,ncheck)){
									//complete file
									if(!strstr(complete_list,name)){
										strcat(complete_list,name);
										strcat(complete_list,"\n");
									}
									flag=1;
								}
							}
						}
					}
				}
				if(!flag){
					if(!strstr(complete_list,name)){
						strcat(complete_list,name);
						strcat(complete_list,"[Incomplete]\n");
					}
				}
			}
			flag=0;
			memset(buff,0,MAX);memset(name,0,MAX);memset(ncheck,0,MAX);
		}

		fd.close();
		cout<<"***** RESULT of LIST *****\n";
		cout<<complete_list<<endl;
		delete name;delete buff;delete ncheck;delete complete_list;
}


void list_files(){
	// send folder if present
	if(!sub_folder[0])
		sub_folder[0]='/';
	int i=0;
	for(i=0;i<NUM_SERVERS;i++)
		send(sockfd[i],sub_folder,sizeof(sub_folder),0);
	// receive filesize and filecontents
	//server 1
	int fs;
	fstream fd;
	char file_n[MAX]="list_file";
	fd.open(file_n,fstream::out|fstream::binary);
	for(i=0;i<NUM_SERVERS;i++){
			recv(sockfd[i],&fs,sizeof(int),0);
			//cout<<"size:"<<fs<<" "<<i+1<<endl;
			char* buf = new char[fs];
			recv(sockfd[i],buf,fs,0);
			encrypt_decrypt_data(buf,fs,password);
			fd.write(buf,fs);
			fs=0;
			delete buf;
	}
	fd.close();

	fd.open(file_n,fstream::in|fstream::binary);
	if(fd.fail())perror("LIST file Open");
	fs=compute_filesize(file_n);
	char *buf1 =new char[fs];
	fd.read(buf1,fs);
	fd.close();
	read_listfile(buf1,file_n,fs);
	delete buf1;
	remove(file_n);


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


  while(1){
				choice=user_menu();
				memset(sockfd,0,sizeof(sockfd));
				create_client_connections();
				//cout<<choice<<endl;
  			if(choice==1){
					// put the file into servers
					if(access_grant(PUT)){
						// divide the file into 4 parts
						if(split_files_in_4(filename)){
							send_files(filename);
							cout<<"Sent files to server in PUT\n";
							system("rm .*");
						}
					}
					else
						cout<<"Invalid Username/Password. Please try again.\n";
					choice=0;
				}
				else if(choice==2){
					if(access_grant(GET)){
						//get all pieces from server
						get_all_pieces();
						system("rm .*");
					}
					else
						cout<<"Invalid Username/Password. Please try again.\n";
					choice=0;
				}
				else if(choice==3){
					if(access_grant(LIST)){
						//list files present in server
						list_files();
					}
					else
							cout<<"Invalid Username/Password. Please try again.\n";
					choice=0;
				}
				else
					cout<<"Wrong command, Enter Again\n";
  }
	return 0;
}
