#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <pthread.h>

#include "main.h"

int create_listener(int port) {

	int sockfd;
	struct sockaddr_in addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
	listen(sockfd, 1);
	printf("Listening on port %d...\n", port);
	return sockfd;
}

int main() {

	int phone_listener = create_listener(PORT_PHONE);
	int iot_listener = create_listener(PORT_IOT);

	printf("Waiting for Phone to connect...\n");
	int phone_fd = accept(phone_listener, NULL, NULL);
	printf("connected!\n");

	printf("Waiting for plane to connect...\n");
	int iot_fd = accept(iot_listener, NULL, NULL);
	printf("connected!\n");


	pthread_t phone_thread, iot_thread;

	ThreadArgs* phone_args = malloc(sizeof(ThreadArgs));
	phone_args->from_fd = phone_fd;
	phone_args->to_fd = iot_fd;
	phone_args->name = "Phone -> IoT";

	ThreadArgs* iot_args = malloc(sizeof(ThreadArgs));
	iot_args->from_fd = iot_fd;
	iot_args->to_fd = phone_fd;
	iot_args->name = "IoT -> Phone";

	pthread_create(&phone_thread, NULL, relay_thread, phone_args);
	pthread_create(&iot_thread, NULL, relay_thread, iot_args);

	pthread_join(phone_thread, NULL);
	pthread_join(iot_thread, NULL);

	close(phone_listener);
	close(iot_listener);

	return 0;
}
