#include "httpd.h"
#include <getopt.h>

#define BUFFERSIZE 256
char dir_path[BUFFERSIZE] = "pc_output";

void help(char *prog)
{
	printf("Usage: %s [-d dir] [-p port]\n", prog);
	printf("\n");
	printf("  -h, --help      display this help\n");
	printf("  -d, --debug     show debug info\n");
	printf("  -t, --target    target static files directory\n");
	printf("  -p, --port      serve port\n");
}

int main(int argc, char **argv)
{
	char serve_port[BUFFERSIZE] = "3000";

	int ret = 0;
	int option_index = 0;
	static int flag = 0;

	static struct option arg_options[] = {
		{"help", no_argument, 0, 'h'},
		{"debug", no_argument, 0, 'd'},
		{"target", required_argument, 0, 't'},
		{"port", required_argument, 0, 'p'},
		{0, 0, 0, 0}
	};

	while ((ret =
		getopt_long(argc, argv, ":hdt:p:", arg_options,
			    &option_index)) != -1) {
		switch (ret) {
		case 'h':
			help(argv[0]);
			return 0;
		case 'd':
			debug_mode = 1;
			break;
		case 't':
			snprintf(dir_path, BUFFERSIZE, "%s", optarg);
			break;
		case 'p':
			snprintf(serve_port, BUFFERSIZE, "%s", optarg);
			break;
		case ':':
			printf("miss argument\n");
			return -1;
		case '?':
			printf("unrecognizable argument\n");
			return -1;
		default:
			return -1;
		}
	}

	serve_forever(serve_port);
	return 0;
}

void route(int sock_fd)
{
	serve_dir(sock_fd, dir_path);
}
