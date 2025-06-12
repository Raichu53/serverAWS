#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

int main ( void ) 
{
    struct sockaddr_in serverInfo;
    
    int sockfd = socket(AF_INET, SOCK_STREAM,0);
    
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_port = htons(34543);
    serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    
    
    if(connect(sockfd,(struct sockaddr*)&serverInfo,sizeof(serverInfo)) < 0)
    {
        printf("connect error: %s\n",strerror(errno));
    }
    
    
    sleep(1);
    
    unsigned char buffer[64] = {
        0xA, 0xBC,0x6f,4,5,6,7,8,9,0xA,0xB,0xC,0xD,0xE,0xF,
        1, 2,3,4,5,6,7,8,9,0xA,0xB,0xC,0xD,0xE,0xF,
        1, 2,3,4,5,6,7,8,9,0xA,0xB,0xC,0xD,0xE,0xF,
        1, 2,3,4,5,6,7,8,9,0xA,0xB,0xC,0xD,0xE,0xF,
        1, 2,0x0D,0xEF
    };
    //printf("temp before : 0x%02x\n",temp);
    //printf("temp after : 0x%02x\n",temp);
    
    while(1){
        int ret = send(sockfd,buffer,64,0);
        printf("sent\n");
        usleep(500000);
    }
    close(sockfd);
    return 0;
}
