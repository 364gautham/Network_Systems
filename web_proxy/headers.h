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
/* It creates socket connection to server and writes to cache*/
void client(int fd,char* host, char* buffer,char* port,char* path,char* http_version);
/*fetches hostname from the received request from browser*/
char* get_hostname(char* str);
/*fetches port number */

char* get_port(char* str);
/* fecthes url path required for data transfer*/
char* get_path(char* str);
/* handles incorrect hostname request*/
void handle_bad_hostname(int fd);
/*handles bad HTTP request*/
void handle_non_GET(int fd);
/*searches for cache file and timeout of cache file*/
int search_f_timeout(char* hash);

/*gets file creation time for caching timeout operation*/
int get_filecreation_time(char* filename);
/* creates new file for caching*/
char* create_file(char* path);
/*checks caching availablity*/
int check_cache(char* path);
/* parse request from browser*/
void request_parse(int fd);
/*to send data stored in cache to user request*/
void send_from_cache(int fd);
/* to block requests for blocked sites*/
void handle_forbidden(int fd);
/* check for blcoked list*/
int blocked_list(char* hostname);
/* to get ip address from hostname for HOSTNAME-IP caching*/
int proxy_hostname_to_ip(char* hostname,char* ip_address);
/* search for hostname in cache */
int search_hostname(char* host,char* ip);
/* interrupt signla handler for gracious exit*/
void signal_handler(int sig);
/* prefecthing operation*/
void fetch_links(char* filename,int fd,char* http_version,char* host);
#endif
