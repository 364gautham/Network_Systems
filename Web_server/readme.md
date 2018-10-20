## HTTP Simple **WEB_SERVER** implementation.

Following HTTP features are implemented:
1.HTTP **GET** request
2.HTTP **POST** request
3.**Pipelining** client request- A client connection will be alive for a fixed time of 10 seconds. If no request is made within ten seconds,
connection would be closed. Timer would be refreshed to 10 seconds if subsequest specific http requests are made.

#Compiling:

Makefile can be used to build the output file.

1. make clean and 2. make would build a web_server output file.

**To run**: ./web_server with specific port number for the client to connect as argument.

In the HTTP request, by default index.html page would be loaded for client if no specific path is provided for the server.

Server has a directory **www/** . It would send successfully all the the file requests from client present in the www/ directory.

#To Test pipeling using telnet command:


In **MAC**: (echo -en "GET /index.html HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n"; sleep 5; echo -en "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n") | nc 127.0.0.1 8888


**NOTE**: 8888 is the port number.**Linux OS** can *telnet* instead of *nc*. Sleep of 5 seconds is given. It should fetch the index.html page twice for successful pipelining operation.

Testing POST request using telnet command:

In **MAC** : (echo -en "POST /index.html HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\nPOSTDATA") | nc 127.0.0.1 8888

For the **Pipelining** feature: **alarm()** function is used which generates a SIGALRM signal after the timer is expired.

For the **POST** feature: Implemented similarly as GET but whatever data present after **POSTDATA** line is added to the header of response between pre tag of HTTP.

**Error Handling** : If requested file is not present or if requested version is not HTTP 1.1/1.0 or not GET or POST HTTP requests , **HTTP/1.1 500 Internal Server Error** is sent to browser client. Client can request again with proper command for successful response.  


