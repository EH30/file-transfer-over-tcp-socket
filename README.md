# File-Transfer-over-tcp-socket   
Send Files Over Tcp Socket in Windows Winsock   

# Compile   
gcc client.c -o client.exe -lws2_32   
gcc server.c -o server.exe -lws2_32   
   
# Usage   
send file to server: ./client.exe [ip] [port] [filename]   
receive file from server: ./client.exe [ip] [port]   
   
send file to client: ./server.exe [ip] [port] [filename]   
receive file from client: ./server.exe [ip] [port]   
