
# xhWebServer
xhWebServer (eXtra Horrible) is a simple webserver written in C, using only standard and POSIX libraries. It targets Linux only.

It is not fully compliant with any standard, but should be good enough for serving HTML.
ALSO REALLY INSECURE but it works!

## Features
- IPv6
- accepts HTTP/0.9, HTTP/1.0 and HTTP/1.1, sends back HTTP/0.9 or HTTP/1.1
- URL decoding
- MIME type detection based on file extension
- built in and custom error pages
- custom CGI implementation (read: no existing CGI scripts work)

## Building
**EDIT options.h FIRST!!!!!!!!** Only touch WEB_ROOT, ERROR_ROOT, SERVER_BANNER, WEB_PORT
Compile all *.c files together.
```bash
Uhhh let me get back to you with that one
```

## Supported formats
| Extension       | MIME type                            |
|-----------------|--------------------------------------|
| .html, .htm     | text/html;charset=utf-8              |
| .css            | text/css;charset=utf-8               |
| .js             | application/javascript;charset=utf-8 |
| .json           | application/json;charset=utf-8       |
| .xml            | application/xml;charset=utf-8        |
| .jpg, .jpeg     | image/jpeg                           |
| .png            | image/png                            |
| .gif            | image/gif                            |
| .svg            | image/svg+xml;charset=utf-8          |
| .webp           | image/webp                           |
| .ico            | image/x-icon                         |
| .mp3            | audio/mpeg                           |
| .ogg            | audio/ogg                            |
| .wav            | audio/wav                            |
| .mid, .midi     | audio/midi                           |
| .mp4            | video/mp4                            |
| .webm           | video/webm                           |
| .pdf            | application/pdf                      |
| .zip            | application/zip                      |
| .woff           | font/woff                            |
| .woff2          | font/woff2                           |
| .ttf            | font/ttf                             |
| .otf            | font/otf                             |
| .txt            | text/plain;charset=utf-8             |
| .csv            | text/csv;charset=utf-8               |
| .\*             | application/octet-stream             |

## CGI environment variables
| Key                 | Value                 |
|---------------------|-----------------------|
| SERVER_SOFTWARE     | xhWebServer/0.0.1     |
| GATEWAY_INTERFACE   | CGI/0                 |
| SERVER_PROTOCOL     | HTTP/0.9, HTTP/1.1    |
| SERVER_PORT         | 12500                 |
| SCRIPT_NAME         | /test.cgi             |
| REMOTE_ADDR         | ::ffff:192.168.68.102 |
| HTTP_METHOD         | GET, POST, HEAD, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH, ERR |

## TODO
- Error checking EVERYWHERE
- Normal CGI support
- Actually implement all HTTP methods
- CLI
- Idk make it better
