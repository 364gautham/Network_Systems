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


void read_dfc_file(const char filename[50]);
void create_client_connections();
int compute_filesize(char filename[30]);
void split_files_in_4(char filename[30]);
void encrypt_decrypt_data(char *buf,int size,char pwd[MAX]);
void send_files(char filename[MAX]);
void send_f(int server_num,int file_piece);
int access_grant(int option);
/* interrupt signla handler for gracious exit*/
void signal_handler(int sig);
#endif
