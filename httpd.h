#ifndef _HTTPD_H___
#define _HTTPD_H___

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#define BUFSIZE 8096
#define FORBIDDEN 403
#define NOTFOUND  404

extern int debug_mode;

//Server control functions

void serve_forever(const char *PORT);

// Client request

char *method,			// "GET" or "POST"
*uri,				// "/index.html" things before '?'
*qs,				// "a=1&b=2"     things after  '?'
*prot;				// "HTTP/1.1"

char *payload;			// for POST
int payload_size;

char *request_header(const char *name);

typedef struct {
	char *name, *value;
} header_t;

static header_t reqhdr[17] = { {"\0", "\0"} };

header_t *request_headers(void);
// user shall implement this function

void route(int sock_fd);

// some interesting macro for `route(int)`
#define ROUTE_START()       if (0) {
#define ROUTE(METHOD,URI)   } else if (strcmp(URI,uri)==0&&strcmp(METHOD,method)==0) {
#define ROUTE_GET(URI)      ROUTE("GET", URI)
#define ROUTE_POST(URI)     ROUTE("POST", URI)
#define ROUTE_REGEX(METHOD,URIREG)   } else if (strncmp(URIREG,uri,strlen(URIREG))==0&&strcmp(METHOD,method)==0) {
#define ROUTE_REGEX_GET(URI)      ROUTE_REGEX("GET", URI)
#define ROUTE_REGEX_POST(URI)     ROUTE_REGEX("POST", URI)
#define ROUTE_END()         } else printf(\
                                "HTTP/1.1 500 Not Handled:[%s]\r\n\r\n" \
                                "The server has no handler to the request.\r\n" \
                            , uri);

static struct {
	char *ext;
	char *filetype;
} extensions[] = {
	{"gif",  "image/gif"},
	{"jpg",  "image/jpg"},
	{"jpeg", "image/jpeg"},
	{"png",  "image/png"},
	{"svg",  "image/svg+xml"},
	{"bmp",  "image/bmp"},
	{"tiff", "image/tiff"},
	{"webp", "image/webp"},
	{"ico",  "image/x-icon"},
	{"zip",  "application/x-zip-compressed"},
	{"gz",   "application/x-gzip"},
	{"tar",  "image/tar"},
	{"bz2",  "application/x-bzip2"},
	{"htm",  "text/html"},
	{"html", "text/html"},
	{"js",   "application/javascript"},
	{"css",  "text/css; charset=utf-8"},
	{"woff", "font/woff"},
	{"woff2", "font/woff2"},
	{"ttf",  "application/x-font-ttf"},
	{"rar",  "application/x-rar-compressed"},
	{"mp4",  "video/mp4"},
	{"mp3",  "audio/mpeg"},
	{"flv",  "video/x-flv"},
	{0, 0}
};

void serve_dir(int sock_fd, char *dirname);
void serve_file(int sock_fd, char *filePath);
#endif
