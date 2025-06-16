#ifndef __MAIN_HEADER__
#define __MAIN_HEADER__

#define	PORT_PHONE	34543
#define	PORT_IOT    34544
#define POSTAMBULE  0x0DEF
#define PREAMBULE   0x0ABC
#define	BUFFER_SIZE 128
#define FROM_BYTE_POS   2
#define FROM_MAIN_BYTE  0xDE
#define FROM_DEVICE_BYTE    0x6F
#define MAX_ARGS    121

#define STOP_SRV    0X69
#define STOP_SRV_P_SZ 4

#define TELEMETRY   0x20
#define TELEMETRY_P_SZ 16

#define MOTOR_SPEED 0x21
#define MOTOR_SPEED_P_SZ 32 


#define BAD_PREAMBULE 1
#define BAD_FROM_BYTE 2
#define BAD_COMMANDID 3
#define BAD_POSTAMBULE 4
#define PAYLOAD_TOO_BIG 5
#define INCOHERENT_PAYLOAD 6

#include <stdint.h>
#include <stdatomic.h>

struct Queue;

typedef struct {
    int from_fd;
    const char* name;
    struct Queue* events;
    pthread_mutex_t lock; 
    atomic_bool* run;
    atomic_bool* isConnected;
} ThreadArgs; 

typedef struct {
    uint16_t    preambule;
    uint8_t     fromByte;
    uint8_t     commandID;
    
    uint8_t     payloadSz;
    uint8_t     payload[MAX_ARGS];
    
    uint16_t    postambule;
}  __attribute__((packed)) Frame;

void routine (pthread_mutex_t* lock, atomic_bool* run,
              struct Queue* q, Frame* f, int fd, const char* target);

#endif //__MAIN_HEADER__
