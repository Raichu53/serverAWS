#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

#include "main.h"
#include "queue.h"
#include "parser.h"

bool parse_buffer ( unsigned char* buffer, Frame* f ) 
{
    int dataSz = 0;
    printf("translating raw buffer to frame\n");
    
    dataSz = sizeof(f->preambule);
    memcpy(&(f->preambule),buffer,dataSz);
    f->preambule = ntohs(f->preambule);
    buffer += dataSz;
    
    dataSz = sizeof(f->fromByte);
    memcpy(&(f->fromByte),buffer,dataSz);
    buffer += dataSz;
    
    dataSz = sizeof(f->commandID);
    memcpy(&(f->commandID),buffer,dataSz);
    buffer += dataSz;
    
    dataSz = sizeof(f->argSz);
    memcpy(&(f->argSz),buffer,dataSz);
    buffer += dataSz;
    
    dataSz = sizeof(uint32_t);
    for(int i = 0 ; i < MAX_ARGS; i++)
    {
        if( i < f->argSz)
        {
            memcpy(&(f->args[i]),buffer,dataSz);
            f->args[i] = ntohl(f->args[i]);
            buffer += dataSz;
        }
        else
        {
            dataSz *= (MAX_ARGS - i);
            memset(&(f->args[i]),0,dataSz);
            buffer += dataSz;
            break;
        }
    }
    
    dataSz = sizeof(f->reserved);
    memset(&(f->reserved),0,dataSz);
    buffer += dataSz;
    
    dataSz = sizeof(f->postambule);
    memcpy(&(f->postambule),buffer,dataSz);
    f->postambule = ntohs(f->postambule);
    buffer += dataSz;
    
    printFrame(f);
    
    return 1;
}

void printFrame( Frame* f )
{
    printf("preambule: \033[0;36m0x%04x\033[0m, fromByte: \033[0;36m0x%02x\033[0m, \
commandID: \033[0;36m0x%02x\033[0m, argSz: \033[0;36m0x%02x\033[0m\n", 
            f->preambule, f->fromByte, f->commandID, f->argSz);
    
    for(int i = 0; i < f->argSz; i++)
    {
        printf("arg\033[0;31m%d\033[0m: \033[0;36m0x%08x\033[0m ",i,f->args[i]);
    }
    printf("\npostambule: \033[0;36m0x%04x\033[0m\n",f->postambule);
}


