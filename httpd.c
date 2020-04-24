#include "httpd.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>

#define CONNMAX 10
#define BACKLOG 20

int debug_mode = 0;
static int listenfd, clients[CONNMAX];
static void error(char *);
static void startServer(const char *);
static void respond(int);

static int clientfd;

static char *buf;

void serve_forever(const char *PORT)
{
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	char c;

	int slot = 0;

	printf("Server started %shttp://127.0.0.1:%s%s\n",
	       "\033[92m", PORT, "\033[0m");

	// Setting all elements to -1: signifies there is no client connected
	int i;
	for (i = 0; i < CONNMAX; i++)
		clients[i] = -1;
	startServer(PORT);

	// Ignore SIGCHLD to avoid zombie threads
	signal(SIGCHLD, SIG_IGN);

	// ACCEPT connections
	while (1) {
		addrlen = sizeof(clientaddr);
		clients[slot] =
		    accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);

		if (clients[slot] < 0) {
			perror("accept() error");
		} else {
			pid_t pid = fork();
			if (pid < 0) {
				perror("fork() error");
			} else if (pid == 0) {
				close(listenfd);
				respond(slot);
				exit(0);
			} else {
				close(clients[slot]);
				clients[slot] = -1;
			}
		}

		while (clients[slot] != -1)
			slot = (slot + 1) % CONNMAX;
	}
}

//start server
void startServer(const char *port)
{
	struct addrinfo hints, *res, *p;

	// getaddrinfo for host
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo(NULL, port, &hints, &res) != 0) {
		perror("getaddrinfo() error");
		exit(1);
	}
	// socket and bind
	for (p = res; p != NULL; p = p->ai_next) {
		int option = 1;
		listenfd = socket(p->ai_family, p->ai_socktype, 0);
		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option,
			   sizeof(option));
		if (listenfd == -1)
			continue;
		if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
			break;
	}
	if (p == NULL) {
		perror("socket() or bind()");
		exit(1);
	}

	freeaddrinfo(res);

	// listen for incoming connections
	if (listen(listenfd, BACKLOG) != 0) {
		perror("listen() error");
		exit(1);
	}
}

// get request header
char *request_header(const char *name)
{
	header_t *h = reqhdr;
	while (h->name) {
		if (strcmp(h->name, name) == 0)
			return h->value;
		h++;
	}
	return NULL;
}

// get all request headers
header_t *request_headers(void)
{
	return reqhdr;
}

//client connection
void respond(int n)
{
	int rcvd, fd, bytes_read;
	char *ptr;

	buf = malloc(65535);
	rcvd = recv(clients[n], buf, 65535, 0);

	if (rcvd < 0)		// receive error
		fprintf(stderr, ("recv() error\n"));
	else if (rcvd == 0)	// receive socket closed
		fprintf(stderr, "Client disconnected upexpectedly.\n");
	else			// message received
	{
		buf[rcvd] = '\0';

		method = strtok(buf, " \t\r\n");
		uri = strtok(NULL, " \t");
		prot = strtok(NULL, " \t\r\n");

		if (debug_mode) {
			fprintf(stderr, "\x1b[32m + [%s] %s\x1b[0m\n", method, uri);
		}

		qs = strchr(uri, '?');
		if (qs) {
			*qs++ = '\0';	//split URI
		} else {
			qs = uri - 1;	//use an empty string
		}

		header_t *h = reqhdr;
		char *t, *t2;
		while (h < reqhdr + 16) {
			char *k, *v, *t;
			k = strtok(NULL, "\r\n: \t");
			if (!k)
				break;
			v = strtok(NULL, "\r\n");
			while (*v && *v == ' ')
				v++;
			h->name = k;
			h->value = v;
			h++;
			if (debug_mode)
				fprintf(stderr, "[H] %s: %s\n", k, v);
			t = v + 1 + strlen(v);
			if (t[1] == '\r' && t[2] == '\n')
				break;
		}
		t++;		// now the *t shall be the beginning of user payload
		t2 = request_header("Content-Length");	// and the related header if there is  
		payload = t;
		payload_size = t2 ? atol(t2) : (rcvd - (t - buf));

		// bind clientfd to stdout, making it easier to write
		clientfd = clients[n];

		// call router
		route(clientfd);
		close(clientfd);
	}

	//Closing SOCKET
	shutdown(clientfd, SHUT_RDWR);	//All further send and recieve operations are DISABLED...
	close(clientfd);
	clients[n] = -1;
}

void logger(int type, char *s1, char *s2, int socket_fd)
{
	int fd;
	char logbuffer[BUFSIZE * 2];

	switch (type) {
	case FORBIDDEN:
		(void)write(socket_fd,
			    "HTTP/1.1 403 Forbidden\nContent-Length: 185\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>403 Forbidden</title>\n</head><body>\n<h1>Forbidden</h1>\nThe requested URL, file type or operation is not allowed on this simple static file webserver.\n</body></html>\n",
			    271);
		(void)sprintf(logbuffer, "FORBIDDEN: %s:%s", s1, s2);
		break;
	case NOTFOUND:
		(void)write(socket_fd,
			    "HTTP/1.1 404 Not Found\nContent-Length: 136\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>Not Found</h1>\nThe requested URL was not found on this server.\n</body></html>\n",
			    224);
		(void)sprintf(logbuffer, "NOT FOUND: %s:%s", s1, s2);
		break;
	}
	return;
}

void serve_file(int sock_fd, char *filePath)
{
	char *fstr = (char *)0;
	int i, ret, file_fd;
	unsigned long len;
	int buflen = strlen(filePath);
	static char buffer[BUFSIZE + 1];	/* static so zero filled */

	char *n = strstr(filePath, "?");
	if (n != NULL) {
		filePath[n - filePath] = '\0';
	}
	for (i = 0; extensions[i].ext != 0; i++) {
		len = strlen(extensions[i].ext);
		if (!strncmp(&filePath[buflen - len], extensions[i].ext, len)) {
			fstr = extensions[i].filetype;
			break;
		}
	}
	//if (fstr == 0)
	//	logger(FORBIDDEN, "file extension type not supported", uri,
	//	       sock_fd);

	if ((file_fd = open(filePath, O_RDONLY)) == -1) {	/* open the file for reading */
		logger(NOTFOUND, "failed to open file", filePath, sock_fd);
		return;
	}
	len = (long)lseek(file_fd, (off_t) 0, SEEK_END);	/* lseek to the file end to find the length */
	(void)lseek(file_fd, (off_t) 0, SEEK_SET);	/* lseek back to the file start ready for reading */
	(void)sprintf(buffer, "HTTP/1.1 200 OK\nServer: nigix/0.0.1\nContent-Length: %ld\nConnection: close\nContent-Type: %s\n\n", len, fstr);	/* Header + a blank line */
	(void)write(sock_fd, buffer, strlen(buffer));

	/* send file in 8KB block - last block may be smaller */
	while ((ret = read(file_fd, buffer, BUFSIZE)) > 0) {
		(void)write(sock_fd, buffer, ret);
	}
	sleep(1);		/* allow socket to drain before signalling the socket is closed */
	close(file_fd);
}

void serve_dir(int sock_fd, char *dirname)
{
	char fp[256];
	ROUTE_START()

	    ROUTE_REGEX_GET("/") {
		if (strcmp(uri, "/") == 0) {
			snprintf(fp, 256, "./%s/index.html", dirname);
		} else {
			snprintf(fp, 256, "./%s%s", dirname, uri);
		}
		serve_file(sock_fd, fp);
	}

	ROUTE_END()
}
