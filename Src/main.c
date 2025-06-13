#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdatomic.h>

#include "main.h"
#include "queue.h"
#include "parser.h"
#include "relayThread.h"

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

    int phone_listener,iot_listener,phone_fd,iot_fd,len = 0;
    
    phone_listener = create_listener(PORT_PHONE);
    iot_listener = create_listener(PORT_IOT);

    printf("\033[0;33mWaiting for Phone to connect...\033[0m\n");
    phone_fd = accept(phone_listener, NULL, NULL);
    printf("\033[0;32mconnected!\033[0m\n");

    printf("\033[0;33mWaiting for plane to connect...\033[0m\n");
    iot_fd = accept(iot_listener, NULL, NULL);
    printf("\033[0;32mconnected!\033[0m\n");

    ThreadArgs* phone_args  = malloc(sizeof(ThreadArgs));
    ThreadArgs* iot_args    = malloc(sizeof(ThreadArgs));
    
    atomic_bool*    run       = malloc(sizeof(atomic_bool));
    pthread_mutex_t planeLock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t phoneLock = PTHREAD_MUTEX_INITIALIZER;
    
    Queue_t*    phone_events      = malloc(sizeof(Queue_t));
    Queue_t*    plane_events      = malloc(sizeof(Queue_t));
    initQueue(phone_events);
    initQueue(plane_events);
    atomic_init(run,1);
    
    if( phone_args != NULL && iot_args != NULL )
    {
        phone_args->from_fd = phone_fd;
        phone_args->name = "Phone";
        phone_args->events = phone_events;
        phone_args->lock = phoneLock;
        phone_args->run = run;
        
        iot_args->from_fd = iot_fd;
        iot_args->name = "Plane";
        iot_args->events = plane_events;
        iot_args->lock = planeLock;
        iot_args->run = run;
        
        pthread_t phone_thread, iot_thread;
        
        pthread_create(&phone_thread, NULL, relay_thread, phone_args);
        pthread_create(&iot_thread, NULL, relay_thread, iot_args);
        
        Frame* f = malloc(sizeof(Frame));
        while(atomic_load(run))
        {
            pthread_mutex_lock(&phoneLock);
            if(!isEmpty(phone_events))
            {
                memset(f,0,sizeof(Frame));
                parse_buffer(*(phone_events->items),f);                
                pop_front(phone_events);
                len = send(iot_fd, f, sizeof(Frame), 0);
                printf("[Main_thread] : %d bytes sent to Plane_thread\n",len);
            }
            pthread_mutex_unlock(&phoneLock);
            
            pthread_mutex_lock(&planeLock);
            if(!isEmpty(plane_events))
            {
                memset(f,0,sizeof(Frame));
                parse_buffer(*(plane_events->items),f);
                pop_front(plane_events);
                len = send(phone_fd, f, sizeof(Frame), 0);
                printf("[Main_thread] : %d bytes sent to Phone_thread\n",len);
            }
            pthread_mutex_unlock(&planeLock);
        }
        free(f);
        
        pthread_join(phone_thread, NULL);
        pthread_join(iot_thread, NULL); 
    }
    
    free(run);
    pthread_mutex_destroy(&planeLock);
    pthread_mutex_destroy(&phoneLock);
    
    close(phone_listener);
    close(iot_listener);
    
    printf("\033[0;32m[Main_thread] : finished successfully\033[0m\n");
    return 0;
}
