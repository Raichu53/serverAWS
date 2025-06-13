#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

int main ( void ) 
{
    struct sockaddr_in serverInfo;
    
    int sockfd = socket(AF_INET, SOCK_STREAM,0);
    
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_port = htons(34544);
    serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    
    
    if(connect(sockfd,(struct sockaddr*)&serverInfo,sizeof(serverInfo)) < 0)
    {
        printf("connect error: %s\n",strerror(errno));
    }
    else
    {
        unsigned char recvb[64];
        int len = 0;
        while(1){
            len = recv(sockfd,recvb,64,0);
            printf("%d byte recv\n",len);
            if(len == 0)
                break;
        }
    }
    
    shutdown(sockfd,2);
    close(sockfd);
    return 0;
}
