#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>

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
    
    unsigned char recvb[64];
    struct timeval t;
    double time1,time2 = 0;
    while(1){
        int ret = send(sockfd,buffer,64,0);
        gettimeofday(&t,NULL);
        time1 = t.tv_sec + 1.0e-3 * t.tv_usec;
        printf("sent\n");
        
//         recv(sockfd,recvb,64,0);
//         gettimeofday(&t,NULL);
//         time2 = t.tv_sec + 1.0e-3 * t.tv_usec;
//         printf("recv\n");
//         
//         
//         printf("%f ms elapsed\n",(double)(time2 - time1));
        
//         break;
        sleep(1);
    }

/*
    //printf("waiting");

    unsigned char buffer2[64];
    ret = recv(sockfd,buffer2,64,0);
    if(ret){
        for(int i = 0 ; i < 64;i++)
        {
            printf("0x%02x ",buffer2[i]);
        }
    }else{

    }
    unsigned char buffer3[64] = {
    0xA, 0xBC,0xde,4,5,6,7,8,9,0xA,0xB,0xC,0xD,0xE,0xF,
    1, 2,3,4,5,6,7,8,9,0xA,0xB,0xC,0xD,0xE,0xF,
    1, 2,3,4,5,6,7,8,9,0xA,0xB,0xC,0xD,0xE,0xF,
    1, 2,3,4,5,6,7,8,9,0xA,0xB,0xC,0xD,0xE,0xF,
    1, 2,0x0D,0xEF
    };
    ret = send(sockfd,buffer3,64,0);
    printf("sent\n");


    unsigned char buffer4[64];
    ret = recv(sockfd,buffer4,64,0);
    if(ret){
        for(int i = 0 ; i < 64;i++)
        {
            printf("0x%02x ",buffer4[i]);
        }
    }else{

    }
    */
    shutdown(sockfd,2);
    close(sockfd);
    return 0;
}
