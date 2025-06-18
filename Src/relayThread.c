#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdatomic.h>
#include <systemd/sd-journal.h>

#include "main.h"
#include "queue.h"
#include "relayThread.h"

/*
 * Main loop des threads
 */
void* relay_thread(void* args) 
{
    ThreadArgs* targs = (ThreadArgs*)args;
    unsigned char buffer[BUFFER_SIZE];
 
    while (atomic_load(targs->run)) 
    {
        bool stop;
        int total = 0;
        memset(buffer,0,BUFFER_SIZE);
        do
        {
            stop = 0;
            //boucle d'attente de reconnexion
            while(0 == atomic_load(targs->isConnected))
            { 
                //verification que le main thread ne se fini pas entre temps
                if(0 == atomic_load(targs->run))
                    break;
                else
                    sleep(1); 
            }
            
            int len = recv(targs->from_fd, buffer + total, BUFFER_SIZE - total, 0);
            
            if(0 >= len)
            {
                //il y a eu une deconnexion, isConnected passe false
                atomic_store(targs->isConnected,0);
                
                if( 0 == len ){
                    sd_journal_print(LOG_ERR,"[%s_thread] :\033[0;31m client not connected\033[0m\n",targs->name);
                }
                else { 
                    sd_journal_print(LOG_CRIT,"[%s_thread] :\033[0;31m unknown error: %s\033[0m\n",targs->name,strerror(errno));
                }
        
                stop = 1; //fin du do-while
            } else {
				total += len; //au cas ou on a pas tout recu d'un coup
            }
			
			if(total == BUFFER_SIZE)
			{
                //on regarde d'ou provient la trame, info sur le byte a l'offset FROM_BYTE_POS
                switch(buffer[FROM_BYTE_POS])
                {
                    case FROM_DEVICE_BYTE:
                        //reception de raw data depuis un des devices (flag 0x6f),
                        //envoie au main via la file des evenements du device
                        pthread_mutex_lock(&(targs->lock));
                        push_back(targs->events,buffer);
                        pthread_mutex_unlock(&(targs->lock));
                        break;
                        
                    case FROM_MAIN_BYTE:
                        //envoie la data analyse par le main thread au device connecte 
                        len = send(targs->from_fd,buffer,BUFFER_SIZE,0);
                    
                        if(0 >= len)
                        {
                            sd_journal_print(LOG_CRIT,"[%s_thread] : \033[0;31msend error: %s\033[0m\n",targs->name,strerror(errno));
                            stop = 1;
                        }
                        else
                        {
                            sd_journal_print(LOG_DEBUG,"[%s_thread] : %d bytes sent to %s_client\n",targs->name,len,targs->name);
                        }
                        break;
                        
                    default:
                        sd_journal_print(LOG_ERR,"[%s_thread] : \033[0;31munknown from_byte value : %x\033[0m\n",
                               targs->name,buffer[FROM_BYTE_POS]);
                        stop = 1;
                        break;
                }
			}
			
        } while(total != BUFFER_SIZE && !stop); //tant que le buffer n'est pas plein + pas de deconnexion
    }

    close(targs->from_fd);
    sd_journal_print(LOG_INFO,"\033[0;32m[%s_thread] : finished successfully\033[0m\n",targs->name);
    free(targs);
    return NULL;
}

int create_listener(int port) {

    int sockfd;
    struct sockaddr_in addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
        sd_journal_print(LOG_CRIT,"socket() failed : %s\n",strerror(errno));
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    const int en = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &en, sizeof(int)) < 0)
        sd_journal_print(LOG_CRIT,"setsockopt() failed : %s\n",strerror(errno));

    if(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        sd_journal_print(LOG_CRIT,"bind() failed : %s\n",strerror(errno));
    
    listen(sockfd, 1);
    sd_journal_print(LOG_DEBUG,"Listening on port %d...\n", port);
    return sockfd;
}
