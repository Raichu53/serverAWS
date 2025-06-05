#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "main.h"

void* relay_thread(void* args) {
	ThreadArgs* targs = (ThreadArgs*)args;
	char buffer[BUFFER_SIZE];

	while (1) {
		int len = recv(targs->from_fd, buffer, BUFFER_SIZE, -1);
		if (len <= 0) {
			printf("[%s] Connection closed or error.\n", targs->name);
			break;
		}
		buffer[len] = '\0';
		printf("[%s] Received: %s\n", targs->name, buffer);
		send(targs->to_fd, buffer, len, 0);
	}

	close(targs->from_fd);
	close(targs->to_fd);
	free(targs);
	return NULL;
}
