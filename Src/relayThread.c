#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdatomic.h>

#include "main.h"
#include "queue.h"

void* relay_thread(void* args) 
{
    ThreadArgs* targs = (ThreadArgs*)args;
    
    unsigned char buffer[BUFFER_SIZE];
    
    while (atomic_load(targs->run)) 
    {
        int total = 0;
        memset(buffer,0,BUFFER_SIZE);
        do
        {
            int len = recv(targs->from_fd, buffer + total, BUFFER_SIZE - total, 0);
            
            if(0 >= len)
            {
                //socket ferme ou erreur inconnue
                printf("socket error: %s\n",strerror(errno));
                break;
            } else 
				total += len;
			
			
			if(total == BUFFER_SIZE)
			{
                if(buffer[FROM_BYTE_POS] ==  FROM_DEVICE_BYTE )
                {
                    pthread_mutex_lock(&(targs->lock));
                    push_back(targs->events,buffer);
                    pthread_mutex_unlock(&(targs->lock));
                }
                else if(buffer[FROM_BYTE_POS] == FROM_MAIN_BYTE )
                {
                    
                }
                else
                {
                    printf("\033[0;31munknown from_byte value : %x\033[0m\n",buffer[FROM_BYTE_POS]);
                }
			}
			
        } while(total != BUFFER_SIZE && total < BUFFER_SIZE);
    }

    close(targs->from_fd);
    free(targs);
    return NULL;
}
