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
#include <dirent.h>
using namespace std;

#define PUT 1
#define GET 2
#define LIST 3
#define MAX 100
#define MAXLINE 1024

/* This function creates socket with given port and localhost address*/
int create_socket(int port);
/* this fucntion is forked to handle client requests*/
void client_handle(int port);
/* this fucntion receives file pieces from client by making user directory*/
void receive_file(int server_num);
/* simple xor encryption */
void encrypt_decrypt_data(char *buf,int size,char pwd[MAX]);
/* reading dfs file for username password authenticity*/
int read_dfs_file();
/* compute size of file in the present directory given fileName*/
int compute_filesize(char filename[MAX]);
/* interrupt signla handler for gracious exit*/
void signal_handler(int sig);
/* returns true if the directory presents : uses path to find particular directory*/
bool is_dir(const char* path);
/* returns true if the file exists in the directory*/
bool fileExists(const char *fileName);
/* helper function for sending files on request from client to mark the files present*/
int get_check(int server_num);
/* sends files to client - uses port number to distinguish between 4 servers */
void send_pfile(int server_num);
/* this fucntion is uses to send the list of files present in the folder*/
void list_files(int server_num);



#endif
