#ifndef __MAIN_HEADER__
#define __MAIN_HEADER__

#define	PORT_PHONE	34543
#define	PORT_IOT	34544
#define	BUFFER_SIZE	sizeof(int)

typedef struct {
    int from_fd;
    const char* name;
} ThreadArgs;

void* relay_thread(void* args);

#endif //__MAIN_HEADER__
