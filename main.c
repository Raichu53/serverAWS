#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <pthread.h>
#include <stdbool.h>

#include "main.h"
#include "queue.h"
#include "parser.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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

    int phone_listener,iot_listener,phone_fd,iot_fd = 0;
    phone_listener = create_listener(PORT_PHONE);
    iot_listener = create_listener(PORT_IOT);

    printf("Waiting for Phone to connect...\n");
    phone_fd = accept(phone_listener, NULL, NULL);
    printf("connected!\n");

    printf("Waiting for plane to connect...\n");
    iot_fd = accept(iot_listener, NULL, NULL);
    printf("connected!\n");

    ThreadArgs* phone_args  = malloc(sizeof(ThreadArgs));
    ThreadArgs* iot_args    = malloc(sizeof(ThreadArgs));
    Queue_t*    events      = malloc(sizeof(Queue_t));
    
    if( phone_args != NULL && iot_args != NULL && events != NULL )
    {
        phone_args->from_fd = phone_fd;
        phone_args->name = "Phone";
        phone_args->events = events;
        iot_args->from_fd = iot_fd;
        iot_args->name = "Plane";
        iot_args->events = events;

        initQueue(events);
        pthread_t phone_thread, iot_thread;
        
        pthread_create(&phone_thread, NULL, relay_thread, phone_args);
        pthread_create(&iot_thread, NULL, relay_thread, iot_args);

        bool run = 1;
        while(run)
        {
            while(!isEmpty(events))
            {
                pthread_mutex_lock(&lock);
                parse_buffer(*(events->items));
                pop_front(events);
                pthread_mutex_unlock(&lock);
            }
        }
        
        pthread_join(phone_thread, NULL);
        pthread_join(iot_thread, NULL); 
        
    }
    
    deleteQueue(events);
    close(phone_listener);
    close(iot_listener);

    return 0;
}
