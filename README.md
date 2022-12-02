## Things works
- flag for debug mode/deamon, log file, cgi-bin folder, ip address, port, web folder
- receiving client request and validate 
    1. Method, Path, Protocol
    2. Headers
- write sample response to client (need to comment out checkPath() though)


## Things to do
1. debug checkPath()
2. We need to discuss about what writer() need, such as information returned from reader(), such as 404, 200, so writer could decide related response
3. writer() need implement cgi-bin
4. writer() need implement sample response for 404, 405, etc.
...
