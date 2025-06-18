#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <systemd/sd-journal.h>

#include "main.h"
#include "queue.h"

void initQueue( Queue_t* q )
{
    q->front = -1;
    q->rear = 0;
    
    for(int i = 0 ; i < MAX_MSG; i++)
    {
        q->items[i] = NULL;
        q->items[i] = malloc(BUFFER_SIZE);
        if(q->items[i] == NULL)
        {
            sd_journal_print(LOG_CRIT,"failed to create queue : %s\n",strerror(errno));
            break;
        }
    }
}

void deleteItems( Queue_t* q )
{
    if(q != NULL)
    {
        for(int i = 0 ; i < MAX_MSG;i++)
        {
            free(q->items[i]);
        }
    }
}

bool isEmpty( Queue_t* q )
{
    return (q->front == (q->rear -1));
}

bool isFull( Queue_t* q )
{
    return (q->rear == MAX_MSG);
}

int getSize( Queue_t* q )
{
    return q->rear;
}

void push_back ( Queue_t* q,  unsigned char* buf )
{
    
    if(!isFull(q))
    {
        memcpy(q->items[q->rear],buf,BUFFER_SIZE);
        q->rear += 1;
    }
}

void pop_front( Queue_t* q )
{
    int sz = getSize(q);
    if(!isEmpty(q))
    {
        for(int i = 1; i < sz; i++)
        {
            q->items[i-1] = q->items[i];
        }
        q->rear -= 1;
    }
}
