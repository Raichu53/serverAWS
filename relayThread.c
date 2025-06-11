#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>

#include "main.h"
#include "queue.h"

void* relay_thread(void* args) 
{
    ThreadArgs* targs = (ThreadArgs*)args;
    
    unsigned char buffer[BUFFER_SIZE];
    int run = 1;
    
    while (run) 
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
                run = 0;
            } else {
				total += len;
			}
			
			if(total == BUFFER_SIZE)
			{
                pthread_mutex_lock(&lock);
                push_back(targs->events,buffer);
                pthread_mutex_unlock(&lock);
			}
			
        } while(total != BUFFER_SIZE && run);
    }

    close(targs->from_fd);
    free(targs);
    return NULL;
}
