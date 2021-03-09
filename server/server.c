#include <stdio.h>
#include <stdlib.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

struct store {
    char buffer[1024];
    char name[1024];
    int size;
    int bytesRecvd;
};

int getSize(char* filename) {
    FILE* fptr;
    int out = 0;
    if ( (fptr = fopen(filename, "rb")) == NULL ) {
        return 1;
    }

    fseek(fptr, 0L, SEEK_END);
    out = ftell(fptr);
    fclose(fptr);    

    return out;
}

int download_file(SOCKET sock){
    FILE* fptr;
    struct store pt;

    pt.bytesRecvd = recv(sock, pt.buffer, sizeof(pt.buffer), 0);
    pt.size = atoi(pt.buffer);
    memset(pt.buffer, 0, sizeof(pt.buffer));
    pt.bytesRecvd = recv(sock, pt.name, sizeof(pt.name), 0);
    fptr = fopen(pt.name, "ab");

    if (fptr == NULL) {
        return 1;
    }

    printf("[*]File name: %s\n", pt.name);
    printf("[*]Size: %d\n", pt.size);

    pt.bytesRecvd = 0;
    while (pt.size > 0) {
        pt.bytesRecvd = recv(sock, pt.buffer, sizeof(pt.buffer), 0);
        pt.size -= pt.bytesRecvd;
        fwrite(pt.buffer, pt.bytesRecvd, 1, fptr);
        memset(pt.buffer, 0, sizeof(pt.buffer));
    }

    printf("[*]Finished\n");
    fclose(fptr);

    return 0;
}

int send_file(SOCKET sock, char* filename) {
    FILE* fptr;
    fptr = fopen(filename, "rb");
    
    if (fptr == NULL) {
        return 1;
    }
    
    int bytesSent = 0;
    char data[1024];

    sprintf(data, "%d", getSize(filename));
    send(sock, data, sizeof(data), 0);
    Sleep(2);
    bytesSent = send(sock, filename, strlen(filename), 0);
    memset(data, 0, sizeof(data));
    Sleep(2);

    while ( (bytesSent = fread(data, 1, sizeof(data), fptr) ) > 0) {
        send(sock, data, bytesSent, 0);
        memset(data, 0, sizeof(data));
    }

    printf("[*]Finished\n");
    fclose(fptr);

    return 0;
}

int server( char*ip, char* filename,  int port ) {

    WSADATA wsadata;
    SOCKET server, client;
    SOCKADDR_IN serveraddr, clientaddr;
    int iresult;
    int bytesRecvd;

    iresult = WSAStartup(MAKEWORD(2, 0), &wsadata);

    if (iresult != 0) {
        printf("error 0");
        WSACleanup();
        return 1;
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.S_un.S_addr = inet_addr(ip);

    server = socket(AF_INET, SOCK_STREAM, 0);

    if (server == INVALID_SOCKET) {
        printf("error 1");
        closesocket(server);
        WSACleanup();
        return 1;
    }

    bind(server, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    iresult = listen(server, 0);

    if (iresult == SOCKET_ERROR) {
        printf("SOCKET_ERROR");
        closesocket(server);
        WSACleanup();
        return 1;
    }

    int clientaddrsize = sizeof(clientaddr);

    while (1) {
        if ((client = accept(server, (SOCKADDR*)&clientaddr, &clientaddrsize)) != INVALID_SOCKET) {
            if (filename == NULL) {
                download_file(client);
            }
            else 
            {
                printf("[*]Sending...\n");
                printf("[*]File name: %s\n", filename);
                send_file(client, filename);
            }
            closesocket(client);
        }
    }

    closesocket(client);
    closesocket(server);
    WSACleanup();
    return 0;
}


int main(int argc, char* argv[])
{
    if ( argc < 3 ) {
        printf("send file : ./server.exe [ip] [port] [filename]\n");
        printf("receive file : ./server.exe [ip] [port]\n");        
        return 1;
    }
    else if ( argc == 3 ) {
        printf("Listening port: %d ip: %s \n", atoi(argv[2]), argv[1]);
        server(argv[1], NULL, atoi(argv[2]));
    }
    else if ( argc == 4 ) {
        printf("Listening port: %d ip: %s \n", atoi(argv[2]), argv[1]);
        server(argv[1], argv[3], atoi(argv[2]));    
    }
    else 
    {
        printf("send file : ./server.exe [ip] [port] [filename]\n");
        printf("receive file : ./server.exe [ip] [port]\n");        
        return 1;    
    }

    return 0;
}
