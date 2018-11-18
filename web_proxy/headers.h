#ifndef _HEADER_H
#define _HEADER_H

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


#define CONNMAX 1000
#define BYTES 1024
#define MAXBUFSIZE 10000

// This Function creates MD5 sum
char* md5sum_create(const char* path);

int create_socket(int port);
void client(int fd,char* host, char* buffer,char* port,char* path,char* http_version);
char* get_hostname(char* str);
char* get_port(char* str);
char* get_path(char* str);
void handle_bad_hostname(int fd);
void handle_non_GET(int fd);
int search_f_timeout(char* hash);
int get_filesize(char* filename);
int get_filecreation_time(char* filename);
char* create_file(char* path);
int check_cache(char* path);
void request_parse(int fd);
void send_from_cache(int fd);
void handle_forbidden(int fd);
int blocked_list(char* hostname);
int proxy_hostname_to_ip(char* hostname,char* ip_address);
int search_hostname(char* host,char* ip);
void s_exit();
#endif
