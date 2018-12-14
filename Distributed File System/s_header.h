#ifndef _S_HEADER_H
#define _S_HEADER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <openssl/md5.h>
#include <time.h>
#include <fstream>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
using namespace std;

#define PUT 1
#define GET 2
#define LIST 3
#define MAX 100
#define MAXLINE 1024


int create_socket(int port);
void client_handle(int port);
void receive_file(int server_num);
void encrypt_decrypt_data(char *buf,int size,char pwd[MAX]);
int read_dfs_file();
int compute_filesize(char filename[MAX]);
/* interrupt signla handler for gracious exit*/
void signal_handler(int sig);
bool is_dir(const char* path);



#endif
