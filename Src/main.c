#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdatomic.h>

#include "main.h"
#include "queue.h"
#include "parser.h"
#include "relayThread.h"

/*
 * Main loop 
 */
int main( void ) {

    int phone_listener,iot_listener = 0;
    
    //creation des ports du serveur
    phone_listener = create_listener(PORT_PHONE);
    iot_listener = create_listener(PORT_IOT);
    
    //initialisation des variables sur le heap pour utilisation entre threads
    ThreadArgs* phone_args  = malloc(sizeof(ThreadArgs));
    ThreadArgs* iot_args    = malloc(sizeof(ThreadArgs));
    
    atomic_bool*    run       = malloc(sizeof(atomic_bool)); //run global
    atomic_init(run,1);
    
    //initialisation des bool sur l'etat des connexions 
    atomic_bool*    bPhoneClient       = malloc(sizeof(atomic_bool)); 
    atomic_init(bPhoneClient,0);
    atomic_bool*    bPlaneClient       = malloc(sizeof(atomic_bool));
    atomic_init(bPlaneClient,0);
    
    //mutex de protection des files d'evenements
    pthread_mutex_t planeLock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t phoneLock = PTHREAD_MUTEX_INITIALIZER;
    
    //creation et initialisation des files d'evenements
    Queue_t*    phone_events      = malloc(sizeof(Queue_t));
    Queue_t*    plane_events      = malloc(sizeof(Queue_t));
    initQueue(phone_events);
    initQueue(plane_events);
    
    if( phone_args != NULL && iot_args != NULL )
    {
        int phone_fd, iot_fd = 0;
        pthread_t phone_thread, iot_thread;
        
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

        pthread_create(&phone_thread, NULL, relay_thread, phone_args);
        pthread_create(&iot_thread, NULL, relay_thread, iot_args);

        Frame* f = malloc(sizeof(Frame));
         
        //boucle principale, gere les connexion des devices a leur thread associe
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
            
            //boucle secondaire, gere les files d'evenements des threads
            while(atomic_load(bPhoneClient) && atomic_load(bPlaneClient) && atomic_load(run))
            {
                routine(&phoneLock, run, phone_events, f, iot_fd, "Phone");
                routine(&planeLock, run, plane_events, f, phone_fd, "Plane");
                
                usleep(MAIN_DELAY_US);
            }
            usleep(LOOP_DELAY_US);
        }
        free(f);
        //pour que les thread s'arretent, il faut les sortirs du recv() bloquant
        unsigned char stopBuffer[BUFFER_SIZE]; 
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
/*
 * fonction routine de depilation des files d'evenements des threads
 * 
 * si il y a des events dans la file, on traduit le raw buffer en type Frame, 
 * en verifiant que les champs de la Frame sont possible
 * 
 * Si la trame est formate correctement, on regarde le commandID pour de potentielles
 * verification logiques supplementaires, puis on envoie au thread du device oppose,
 * en precisant que la frame provient du main (FROM_MAIN_BYTE)
 */
void routine (pthread_mutex_t* lock, atomic_bool* run,
              struct Queue* q, Frame* f, int fd, const char* target)
{
    int ret = 1;
    
    pthread_mutex_lock(lock);
    if(!isEmpty(q))
    {
        memset(f,0,sizeof(Frame));
        ret = parse_buffer(*(q->items),f);
        pop_front(q);
    } 
    pthread_mutex_unlock(lock);
    
    if(0 == ret)
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
