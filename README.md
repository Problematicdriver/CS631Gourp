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

## NAME
**sws** — a simple web server

## SYNOPSIS
**sws** is a very simple web server. It behaves quite like you would expect from any regular web server, in that it binds to a given port on the given address and waits for incoming HTTP/1.0 requests. It serves content from the given directory. That is, any requests for documents is resolved relative to this directory (the docu- ment root).

## OPTIONS
The following options are supported by sws:
```
−c dir −d
−h
−i address
−l file −p port
```

## DETAILS
**sws** speaks a crippled dialect of HTTP/1.0: it responds to GET and HEAD requests according to RFC1945, and only supports the If-Modified-Since Request-Header.

When a connection is made, sws will respond with the appropriate HTTP/1.0 status code and the following headers:

```
Date            The current timestamp in GMT.
Server          A string identifying this server and version.
Last-Modified   The timestamp in GMT of the file’s last modification date.
Content-Type    text/html or, optionally, the correct content-type for the file in question as deter- mined via magic(5) patterns.
Content-Length  The size in bytes of the data returned.
```

If the request type was a GET, then it will subsequently return the data of the requested file. After serving the request, the connection is terminated.

## FEATURES
**sws** supports a number of interesting features:

- **CGIs**: Execution of CGIs as described in CGIs.
- **Directory Indexing**: If the request was for a directory and the directory does not contain a file named "index.html", then sws will generate a directory index, listing the con- tents of the directory in alphanumeric order. Files starting with a "." are ignored
- **User Directories**: If the request begins with a " ̃", then the following string up to the first slash is translated into that user’s sws directory (ie /home/<user>/sws/)

## CGIs
If a URI begins with the string "/cgi-bin", and the −c flag was specified, then the remainder of the resource path will be resolved relative to the directory specified using this flag. The resulting file will then be executed and any output generated is returned instead. Execution of CGIs follows the specification in RFC3875.
    
## LOGGING
Per default, sws does not do any logging. If explicitly enabled via the −l flag, sws will log every request in a slight variation of Apache’s so-called "common" format: ’%a %t "%r" %>s %b’. That is, it will log:
- %a  The remote IP address.
- %t  The time the request was received (in GMT).
- %r  The (quoted) first line of the request.
- %>s The status of the request.
- %b  Size of the response in bytes. Ie, "Content-Length".
    
Example log lines might look like so:
```
155.246.89.8 2019-10-28T12:12:12Z "GET / HTTP/1.0" 200 1406
2001::fe72:3900 2019-10-28T12:12:12Z "GET /file HTTP/1.0" 404 312
```
All lines will be appended to the given file unless −d was given, in which case all lines will be printed to std- out.

## EXAMPLE INVOCATIONS
```
sws -p 8081 -c cgi-bin docroot
```
## EXIT STATUS
The **sws** utility exits 0 on success, and >0 if an error occurs.

## SEE ALSO
[RFC1945](https://www.ietf.org/rfc/rfc1945.txt)

## HISTORY
A simple http server has been a frequent final project assignment for the class Advanced Programming in the UNIX Environment at Stevens Institute of Technology. This variation was first assigned in the Fall 2008 by Jan Schaumann.
