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

int main() {

    int phone_listener,iot_listener,phone_fd,
        iot_fd = 0;
    
    phone_listener = create_listener(PORT_PHONE);
    iot_listener = create_listener(PORT_IOT);

    ThreadArgs* phone_args  = malloc(sizeof(ThreadArgs));
    ThreadArgs* iot_args    = malloc(sizeof(ThreadArgs));
    
    atomic_bool*    run       = malloc(sizeof(atomic_bool));
    atomic_init(run,1);
    atomic_bool*    bPhoneClient       = malloc(sizeof(atomic_bool));
    atomic_init(bPhoneClient,0);
    atomic_bool*    bPlaneClient       = malloc(sizeof(atomic_bool));
    atomic_init(bPlaneClient,0);
    pthread_mutex_t planeLock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t phoneLock = PTHREAD_MUTEX_INITIALIZER;
    
    Queue_t*    phone_events      = malloc(sizeof(Queue_t));
    Queue_t*    plane_events      = malloc(sizeof(Queue_t));
    initQueue(phone_events);
    initQueue(plane_events);
    
    if( phone_args != NULL && iot_args != NULL )
    {
        phone_args->name = "Phone";
        phone_args->events = phone_events;
        phone_args->lock = phoneLock;
        phone_args->run = run;
        phone_args->isConnected = bPhoneClient;

        iot_args->name = "Plane";
        iot_args->events = plane_events;
        iot_args->lock = planeLock;
        iot_args->run = run;
        iot_args->isConnected = bPlaneClient;

        pthread_t phone_thread, iot_thread;

        pthread_create(&phone_thread, NULL, relay_thread, phone_args);
        pthread_create(&iot_thread, NULL, relay_thread, iot_args);

        Frame* f = malloc(sizeof(Frame));
                
        while(atomic_load(run))
        {
            
            if( 0 == atomic_load(bPhoneClient)) 
            { 
                printf("\033[0;33mWaiting for Phone to connect...\033[0m\n");
                phone_fd = accept(phone_listener, NULL, NULL);
                printf("\033[0;32mPhone connected!\033[0m\n");
                phone_args->from_fd = phone_fd;
                atomic_store(bPhoneClient,1);
            }
            if( 0 == atomic_load(bPlaneClient)) 
            {
                printf("\033[0;33mWaiting for plane to connect...\033[0m\n");
                iot_fd = accept(iot_listener, NULL, NULL);
                printf("\033[0;32mPlane connected!\033[0m\n");
                iot_args->from_fd = iot_fd;
                atomic_store(bPlaneClient,1);
            }
            
            while(atomic_load(bPhoneClient) && atomic_load(bPlaneClient) && atomic_load(run))
            {
                routine(&phoneLock,run,phone_events,f,iot_fd, "Phone");
                routine(&planeLock,run,plane_events,f,phone_fd, "Plane");
                
                usleep(1000);
            }
            usleep(1000);
        }
        free(f);
        
        unsigned char stopBuffer[BUFFER_SIZE]; //pour sortir du recv() bloquant
        memset(stopBuffer,0,BUFFER_SIZE);
        
        send(iot_fd, stopBuffer, BUFFER_SIZE,0);
        pthread_join(iot_thread, NULL); 
        send(phone_fd, stopBuffer, BUFFER_SIZE,0);
        pthread_join(phone_thread, NULL);
    }

    free(run);
    free(bPhoneClient);
    free(bPlaneClient);
    pthread_mutex_destroy(&planeLock);
    pthread_mutex_destroy(&phoneLock);
    
    close(phone_listener);
    close(iot_listener);
    
    printf("\033[0;32m[Main_thread] : finished successfully\033[0m\n");
    return 0;
}

void routine (pthread_mutex_t* lock, atomic_bool* run,
              struct Queue* q, Frame* f, int fd, const char* target)
{
    int ret,len = 0;
    
    pthread_mutex_lock(lock);
    if(!isEmpty(q))
    {
        memset(f,0,sizeof(Frame));
        ret = parse_buffer(*(q->items),f);
        pop_front(q);
        
        if(!ret)
        {
            bool status = 1;
            
            if(f->commandID == STOP_SRV) {
                printf("stop command received, ending...\n");
                atomic_store(run,0);
                status = 0;
            }
            else if(f->commandID == TELEMETRY) {
                //faire des tests par rapports au valeurs precedentes, etc...
            }
            else if(f->commandID == MOTOR_SPEED) {
                //faire des tests par rapports au valeurs precedentes, etc...
            }
            
            if(status) { 
                f->fromByte = FROM_MAIN_BYTE;
                int len = send(fd, f, sizeof(Frame), 0);
                printf("[Main_thread] : %d bytes sent to %s_thread\n", len, target);
            }
        }
    }
    
    pthread_mutex_unlock(lock);
}
