#ifndef _HEADERS_H
#define _HEADERS_H

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
using namespace std;

#define PUT 1
#define GET 2
#define LIST 3
#define INVALID 0
#define NUM_SERVERS 4
#define MAXLINE 1024
#define MAX 100

/* reading configuration file for server , username and password information*/
void read_dfc_file(const char filename[50]);
/*creates socket connection to servers , stores socket descriptor*/
void create_client_connections();
/* computes filesize given filename*/
int compute_filesize(char filename[30]);
/* splits files into 4 to be put into 4 different servers*/
int split_files_in_4(char filename[30]);
/* xor encryption*/
void encrypt_decrypt_data(char *buf,int size,char pwd[MAX]);
/* helper fucntion for sending file pieces to server - hash mod piece table*/
void send_files(char filename[MAX]);
/* helper for sending fucntion*/
void send_helper(int server_num,int file_piece);
/* to check for access to server*/
int access_grant(int option);
/* interrupt signla handler for gracious exit*/
void signal_handler(int sig);
/* to combine file pieces received from server*/
void combine_files();
/* list files prsent in the server*/
void list_files();
/* to combine pieces to display complete or imcomplete file for output of list function*/
void read_listfile(char* buf, char fname[MAX],int fs);

#endif
