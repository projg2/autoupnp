#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include <pthread.h>

int EXIT_INDETERM = 126;
int EXIT_SUCC = EXIT_SUCCESS; /* EXIT_SUCCESS can be a macro */

void* thread_main(void* unused) {
	const int repeat_count = 4;
	static int port = 34567;
	int rep;

	usleep((rand() % 100) * 10000);

	for (rep = 0; rep < repeat_count; rep++) {
		int fd = socket(AF_INET, SOCK_STREAM, 0);
		struct sockaddr_in addr;
		int ret;

		if (!fd) {
			fprintf(stderr, "socket() failed: %s\n", strerror(errno));
			return &EXIT_INDETERM;
		}

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		do {
			addr.sin_port = htons(port);
			ret = bind(fd, &addr, sizeof(addr));
			if (ret && (port >= 34627 || (errno != EADDRINUSE && errno != EACCES))) {
				fprintf(stderr, "bind() failed (last tried port: %d): %s\n",
						port, strerror(errno));
				return &EXIT_INDETERM;
			}
			port++;
		} while (ret);

		if (listen(fd, 16)) {
			fprintf(stderr, "listen() failed: %s\n", strerror(errno));
			return &EXIT_INDETERM;
		}

		usleep((rand() % 100) * 10000);

		close(fd);
	}

	return &EXIT_SUCC;
}

int main(void) {
	const int thread_count = 4;
	pthread_t threads[thread_count];
	int i;

	srand(time(NULL));

	for (i = 0; i < thread_count; i++)
		pthread_create(&(threads[i]), NULL, &thread_main, NULL);

	for (i = 0; i < thread_count; i++) {
		int* ret;
		pthread_join(threads[i], &ret);
		if (*ret != EXIT_SUCCESS)
			return *ret;
	}

	return EXIT_SUCCESS;
}
