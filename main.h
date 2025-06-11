#ifndef __MAIN_HEADER__
#define __MAIN_HEADER__

#define	PORT_PHONE	34543
#define	PORT_IOT	34544
#define	BUFFER_SIZE	64

struct Queue;

typedef struct {
    int from_fd;
    const char* name;
    struct Queue* events;
} ThreadArgs;

extern pthread_mutex_t lock; 

void* relay_thread(void* args);

#endif //__MAIN_HEADER__
