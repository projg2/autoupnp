#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>

const int EXIT_INDETERM = 126;

int main(void) {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr;
	int port = 34567;
	int ret, nfd;

	if (!fd) {
		fprintf(stderr, "socket() failed: %s\n", strerror(errno));
		return EXIT_INDETERM;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	do {
		addr.sin_port = htons(port);
		ret = bind(fd, &addr, sizeof(addr));
		if (ret && (port >= 34587 || (errno != EADDRINUSE && errno != EACCES))) {
			fprintf(stderr, "bind() failed (last tried port: %d): %s\n",
					port, strerror(errno));
			return EXIT_INDETERM;
		}
		port++;
	} while (ret);

	if (listen(fd, 16)) {
		fprintf(stderr, "listen() failed: %s\n", strerror(errno));
		return EXIT_INDETERM;
	}

	if (fork() == -1) {
		fprintf(stderr, "fork() failed: %s\n", strerror(errno));
		return EXIT_INDETERM;
	}
	close(fd);

	nfd = open("/dev/null", O_RDONLY);
	close(nfd);
	if (fd != nfd) {
		fprintf(stderr, "libc didn't give the same fd (%d != %d)\n", fd, nfd);
		return EXIT_INDETERM;
	}

	return EXIT_SUCCESS;
}
