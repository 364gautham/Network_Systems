# Documentation for UDP client server reliable transfer implementation

This program uses UDP - Connection less sockets for file transfer and implements reliability around this protocol.
UDP is by default a not un-reliable transport layer protocol used for speed and real time services like video services.

We have implemented reliability around it without keeping in mind the time[ user interaction real time service].

For this implementation, I have used stop and wait protocol acknowledgement process. Here each packet is acknowledged by receiver and then
the sender sends a new packet. If their is a drop in the process same packet is sent from sender until it receives acknowledgement of packet receive. To keep track of packets I have used sequence number.

So if their is drop in sender to receiver the receiver will not receive packet and will not be sending any ack or nack. Since the sender has time out
implemented in to socket it timeout and sends the packet again to receive ack.
If their is a packet drop in from receiver to sender while sending the ack , the sender would send the same packet again and after being recieved by receiver
it drops the packet and sends the ack with the sequence number.

Socket timeout is hard coded to 500ms.

The client program can send and receive files from server. It can get the list of files present in server and request for any file.
It has a feature to delete a file present in server by sending its filename. It can exit the server safely and client cam be exited by ctrl-c interrupt signal.

Build Process:

Makefile is created for the compilation for server and client programs.

make all : to build the files.
make clean : removes the object files created.

Usage Details:

objects files created can be run using :
for client : ./client [ip_address] [port_number]
for Server: ./server [port_number]

for running in the same system use "localhost" for ip address. Use port numbers greater than 5000 [registered ports: ordinary users]

Put File :

User can type put followed by filename he wantes to send send across. User has to enter the enter files in current directory.

example : "put foo1"

1. Sender would send the option selected to reciever[put,get,ls,delete,exit]
2. Sender sends the filename and filesize to receiver.
3. Sender receives from reciever ready command to start the file transfer.
4. Sender and receiver would be in while loop till the file size is zero.

Get file:

User can type - for eg: "get foo1" - to get the file from the server directory.

1.Receiver sends the option to sender.
2.Receiver will receive the file name and filesize foe transfer initiated.
3.Sender would send the file until the size is zero which is recieved and is written into filesytem by the receiver.


ls:
Client would request the list of files in the sender and prints them at the client side.

eg usage: "ls"

Delete file :
client can request the server to delete the file by sending the filename. It deletes the files if present and sends the ack.

eg usage. "delete foo1"

exit:
Client can request the server to exit the process. eg usage: "exit"
