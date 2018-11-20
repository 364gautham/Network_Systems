# Web_Proxy Server
* Proxy server that has cache timeout of 60 seconds and can block sites that are blacklisted.


## Running Instructions 
* proxy.cpp is the source file. for compiling, type: make all in the folder
* to run the server: ./proxy <port> <timeout for cache storage>
* Browser has to be set to connect to internet through proxy server to get features of proxy
* make clean would clear the compiled file


## Description

This proxy server accepts GET HTTP protocol requests. It checks for validity of Host and returns error 404 for bad request.
If the request is not GET, it returns error 400- Bad request.
If the site is Blocked , which can be specified in the blocked list file, if the request comes for vlocked site : Error 403 : Forbidden site is sent to browser.

It saves the Hostname to IP mapping in the cache at the first request to that site through the DNS query. For the later requests, validity of hostname and DNS queries are removed and hence it saves lot of time in delivering information to user.

It also caches the site information with an expiry time of 60 seconds. If the request for the same site is made within timeout, request is served from the cache without requesting from root server.

It also fetches the HTTP links in the site in advance and caches them for faster delivery to user.





