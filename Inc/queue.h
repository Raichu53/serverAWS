#ifndef __QUEUE_HEADER__
#define __QUEUE_HEADER__

#include <stdbool.h>

#define MAX_MSG 24

typedef struct Queue {
    unsigned char*   items[MAX_MSG];
    int front,rear;
} Queue_t;

void initQueue( Queue_t* q );
void deleteQueue( Queue_t* q );
bool isEmpty( Queue_t* q );
bool isFull( Queue_t* q );
int getSize( Queue_t* q );
void push_back ( Queue_t* q,  unsigned char* buf );
void pop_front( Queue_t* q );

#endif //__QUEUE_HEADER__
