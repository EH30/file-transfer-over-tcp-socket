/*
* Created by: EH
*
*/

#include <stdio.h>
#include <ws2spi.h>
#include <winsock2.h>
#include <windows.h>

struct store {
    char buffer[1024];
    char name[1024];
    int size;
    int bytesRecvd;
};

int wsaStart() {
    WSADATA data;
    WORD ver = MAKEWORD(2, 2);
    int wsResult = WSAStartup(ver, &data);
    if (wsResult != 0) {
        printf("error");
        return 1;
    }
    return 0;
}

SOCKET sockConnect(){
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("ERROR");
        return 1;
    }
    return sock;
}

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

int last_pos(char buff[]) {
    for ( int i = strlen(buff); i >= 0 ; i-- ) {
        if ( buff[i] == '\\' || buff[i] == '/' ) {
            return i;
        }
    }

    return -1;
}

char * Get_Name(char *filepath, char name[], int length) {
    int count = 0;
    int pos = last_pos(filepath);

    if ( pos < 0 ) {
        return filepath;
    }

    for (int i = pos+1; i < length; i++ ) {
        name[count] = filepath[i];
        count++;
    }
    return name;
}

int download_file(SOCKET sock){
    FILE* fptr;
    struct store pt;
    
    pt.bytesRecvd = recv(sock, pt.buffer, sizeof(pt.buffer), 0);
    pt.size = atoi(pt.buffer);
    memset(pt.buffer, 0, sizeof(pt.buffer));
    pt.bytesRecvd = recv(sock, pt.name, sizeof(pt.name), 0);
    fptr = fopen(pt.name, "wb");

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
    
    send(sock, "\n\n\n", sizeof("\n\n\n"), 0);
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
    char buffer[1024];

    sprintf(buffer, "%d", getSize(filename));
    send(sock, buffer, sizeof(buffer), 0);
    printf("[*]Size: %s\n", buffer);
    Sleep(2000);

    memset(buffer, 0, 1024);
    sprintf(buffer, "%s", Get_Name(filename, buffer, strlen(filename)));

    bytesSent = send(sock, buffer, strlen(buffer)+1, 0);
    memset(buffer, 0, sizeof(buffer));
    Sleep(2000);

    memset(buffer, 0, 1024);
    while ( (bytesSent = fread(buffer, 1, sizeof(buffer), fptr) ) > 0) {
        send(sock, buffer, bytesSent, 0);
        memset(buffer, 0, sizeof(buffer));
    }

    printf("[*]Waiting for response...\n");
    recv(sock, buffer, sizeof(buffer), 0);

    printf("[*]Finished\n");
    fclose(fptr);

    return 0;
}

int ClientSoc(char* IP , char* filename, int port) {
    wsaStart();
    SOCKET sock = sockConnect();

    SOCKADDR_IN hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    hint.sin_addr.s_addr = inet_addr(IP);

    int conResult = connect(sock, (SOCKADDR*)&hint, sizeof(hint));
    if (conResult == SOCKET_ERROR) {
        printf("error");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    if ( filename == NULL) {
        download_file(sock);
    }
    else 
    {
        printf("[*]Sending...\n");
        printf("[*]File name: %s\n", filename);
        send_file(sock, filename);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}

int main(int argc, char *argv[]) {
    if ( argc < 3 ) {
        printf("send file : ./client.exe [ip] [port] [filename]\n");
        printf("receive file : ./client.exe [ip] [port]\n");
        return 1;
    }
    else if ( argc == 3 ) {
        ClientSoc(argv[1], NULL, atoi(argv[2]));
    }
    else if ( argc == 4 ) {
        ClientSoc(argv[1], argv[3], atoi(argv[2]));
    }
    else 
    {
        printf("send file : ./client.exe [ip] [port] [filename]\n");
        printf("receive file : ./client.exe [ip] [port]\n");
        return 1;
    }
    
    return 0;
}
