#ifndef __MAIN_HEADER__
#define __MAIN_HEADER__

#define	PORT_PHONE	34543
#define	PORT_IOT    34544
#define POSTAMBULE  0x0ABC
#define PREAMBULE   0x0DEF
#define	BUFFER_SIZE 64
#define FROM_BYTE_POS   2
#define FROM_MAIN_BYTE  0xDE
#define FROM_DEVICE_BYTE    0x6F
#define MAX_ARGS    14


#include <stdint.h>
#include <stdatomic.h>

struct Queue;

typedef struct {
    int from_fd;
    const char* name;
    struct Queue* events;
    pthread_mutex_t lock; 
    atomic_bool* run;
} ThreadArgs; 

typedef struct {
    uint16_t    preambule;
    uint8_t     fromByte;
    uint8_t     commandID;
    uint8_t     argSz;
    uint32_t    args[MAX_ARGS];
    
    uint8_t     reserved;
    uint16_t    postambule;
}  __attribute__((packed)) Frame;

void* relay_thread(void* args);

#endif //__MAIN_HEADER__
