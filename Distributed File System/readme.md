# Distributed File System
 Distributed file system for reliable and secure file storage.
 It is client/server based application.
 Client has various commands like PUT, GET and LIST for file transfer operation between server and Client.
 Client Transfer is done by dividing the file into 4 pieces and is made redundant in nature, as 4 servers in total will have one extra copy of file.
 Client interacts with all the live servers, even if one them disconnects in between.
 Multiple clients can access the server at the same time.
 Data transfer is made secure by performing xor encryption.
 Each client has Configuration information which has information on username and password, It can access servers services if the username and password is authenticated by server for every file transfer operation.

 Client creates a hash value for each file content and gets a mod value.This mod value is used for finding what pieces go into which server.

## Running Instructions
* `make all` - would compile client and server application programs
* To Run Server: `./server <port num> <dfs.conf>`
* To Run Client `./client <dfc.conf>`
## Files
* `dfc.conf` : Configuration details for Client
* `dfs.conf` :Configuration details for Server

## Commands:

# PUT <filename> <subfolder -[optional]>
The filename entered after put is put into each server. Entering sub folder is optional. By default it puts the file into username folder which gets created for each client.

# GET <filename> <subfolder -[optional]>

Gets the filename from the servers. If all the servers are not alive or if all the pieces for constructing the file are not present, it cannot complete this operation.

# LIST <subfolder -[optional]>
list all the files in the main username folder. If subfolder is given it lists all the files in that particular folder.
It lists complete and incomplete files that are present in all servers combined.
