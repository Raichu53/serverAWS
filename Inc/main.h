#ifndef __MAIN_HEADER__
#define __MAIN_HEADER__

#define MAIN_DELAY_US  1000
#define LOOP_DELAY_US  1000

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
    int from_fd; //socket vers le device
    const char* name; //nom du device
    struct Queue* events;//file des evements du device
    pthread_mutex_t lock; //mutex de la file des evements
    atomic_bool* run; //run du programme entier
    atomic_bool* isConnected; //si le device est connecte 
} ThreadArgs; 

typedef struct {
    uint16_t    preambule; //signal le debut d'une trame, tjs 0x0ABC
    uint8_t     fromByte; //origine de la trame, pour que les threads envoie au main ou au device
    uint8_t     commandID; //l'identifiant d'une commande
    
    uint8_t     payloadSz; //la taille du payload (lie au commandID)
    uint8_t     payload[MAX_ARGS];//le buffer de taille fixe
    
    uint16_t    postambule; //signal la fin d'une trame, tjs 0x0DEF
}  __attribute__((packed)) Frame; //informer le compilateur de ne pas arrondir les variables a 4B

void routine (pthread_mutex_t* lock, atomic_bool* run,
              struct Queue* q, Frame* f, int fd, const char* target);

#endif //__MAIN_HEADER__
