#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <systemd/sd-journal.h>

#include "main.h"
#include "queue.h"
#include "parser.h"

/*
 * Cette fonction traduit un raw buffer au format Frame
 * effectue les verifications de formatage
 * 
 * @return  0 si OK
 *          1 si erreur
 */
bool parse_buffer ( unsigned char* buffer, Frame* f ) 
{
    bool status = 0;
    int dataSz,ret = 0;
    
    dataSz = sizeof(f->preambule);
    memcpy(&(f->preambule),buffer,dataSz);
    buffer += dataSz;
    
    dataSz = sizeof(f->fromByte);
    memcpy(&(f->fromByte),buffer,dataSz);
    buffer += dataSz;
    
    dataSz = sizeof(f->commandID);
    memcpy(&(f->commandID),buffer,dataSz);
    buffer += dataSz;
    
    dataSz = sizeof(f->payloadSz);
    memcpy(&(f->payloadSz),buffer,dataSz);
    buffer += dataSz;
    
    dataSz = f->payloadSz;
    if(dataSz < MAX_ARGS)
    {
        memcpy(&(f->payload),buffer,dataSz);
        memset((&(f->payload)) + dataSz, 0, (MAX_ARGS - dataSz));
    }
    else
    {
        memset(&(f->payload),0,MAX_ARGS);
    }
    buffer += MAX_ARGS;
    
    dataSz = sizeof(f->postambule);
    memcpy(&(f->postambule),buffer,dataSz);
    buffer += dataSz;
    
    ret = checkFrame(f);
    if(ret)
    {
        sd_journal_print(LOG_ERR,"[Main_thread] :\033[0;31m checkFrame returned error code %d (%s)\033[0m\n",
               ret,codeTostring(ret));
        printFrame(f);
        status = 1;
    }
    
    return status;
}
/*
 * verifie les champs de Frame et associe un code d'erreur
 * @return  0 si OK
 *          >0 si erreur
 */
uint8_t checkFrame( Frame* f )
{
    uint8_t status = 0;
    if(ntohs(f->preambule) != PREAMBULE) status = BAD_PREAMBULE;
    
    else if(f->fromByte != FROM_DEVICE_BYTE) status = BAD_FROM_BYTE;
    
    else if(0 == isCMDidValid(f->commandID)) status = BAD_COMMANDID;
    
    else if(MAX_ARGS < f->payloadSz) status = PAYLOAD_TOO_BIG;
    
    else if(0 == checkParamSz(f->commandID,f->payloadSz)) status = INCOHERENT_PAYLOAD;
    
    else if(ntohs(f->postambule) != POSTAMBULE) status = BAD_POSTAMBULE;
 
    return status;
}
/*
 * traduit un code d'erreur en entier vers une string
 * @return  la string associee si OK
 *          "UNKNOWN" si erreur
 */
const char* codeTostring(uint8_t errCode)
{
    switch(errCode)
    {
        case BAD_PREAMBULE:
            return "BAD_PREAMBULE";
        case BAD_FROM_BYTE:
            return "BAD_FROM_BYTE";
        case BAD_COMMANDID:
            return "BAD_COMMANDID";
        case BAD_POSTAMBULE:
            return "BAD_POSTAMBULE";
        case PAYLOAD_TOO_BIG:
            return "PAYLOAD_TOO_BIG";
        case INCOHERENT_PAYLOAD:
            return "INCOHERENT_PAYLOAD";
        default:
            break;
    }
    return "UNKNOWN";
}
/*
 * verifie que le command ID est existant
 * @return  1 si OK
 *          0 si erreur
 */
bool isCMDidValid(uint8_t cID) 
{
    bool status = 1;
    switch(cID)
    {
        case TELEMETRY:
            break;
        case MOTOR_SPEED:
            break;
        case STOP_SRV:
            break;
        default:
            status = 0;
            break;
    }
    return status;
}
/*
 * compare le champ payloadSz recu au payload associe
 * a la commande 
 * @return  1 si OK
 *          0 si erreur
 */
bool checkParamSz( uint8_t cID, uint8_t sz ) 
{
    bool status = 1;
    
    switch (cID)
    {
        case TELEMETRY:
            if(sz != TELEMETRY_P_SZ)
                status = 0;
            break;
        case MOTOR_SPEED:
            if(sz != MOTOR_SPEED_P_SZ)
                status = 0;
            break;
        case STOP_SRV:
            if(sz != STOP_SRV_P_SZ)
                status = 0;
            break;
        default:
            status = 0;
            break;
    }
    
    return status;
}
/*
 * print dans le terminal tout le contenu d'une Frame
 */
void printFrame( Frame* f )
{
    sd_journal_print(LOG_DEBUG,"preambule: \033[0;36m0x%04x\033[0m, fromByte: \033[0;36m0x%02x\033[0m, \
commandID: \033[0;36m0x%02x\033[0m, payloadSz: \033[0;36m0x%02x\033[0m\n", 
            f->preambule, f->fromByte, f->commandID, f->payloadSz);
    
    for(int i = 0; i < f->payloadSz; i++)
    {
        sd_journal_print(LOG_DEBUG,"\033[0;36m0x%02x\033[0m ",f->payload[i]);
    }
    sd_journal_print(LOG_DEBUG,"\npostambule: \033[0;36m0x%04x\033[0m\n",f->postambule);
}


