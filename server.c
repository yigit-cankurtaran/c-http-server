#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080 // on localhost:8080
#define BUFFER_SIZE 1024

struct sockaddr_in address;
socklen_t addrlen = sizeof(address);

int create_socket()
{
    int server_fd;
    // fd = file descriptor, number that refers to an open file, socket, or pipe
    // socket() creates an endpoint for communication and returns a file descriptor that refers to that endpoint
    // AF_INET = IPv4 internet protocols
    // SOCK_STREAM = TCP
    // 0 = default protocol for stream sockets
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        return -1;
    }
    printf("Socket created on port %d\n", PORT);
    return server_fd;
}

void bind_socket(int server_fd)
{

    memset(&address, '0', sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket bound to port %d\n", PORT);
}

int client_connect(int server_fd)
{
    int client_fd;
    if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        perror("accept");
        return -1;
    }
    printf("Connection accepted on port %d\n", PORT);
    return client_fd;
}

int main(int argc, char *argv[])
{
    int server_fd = create_socket();
    bind_socket(server_fd);
    client_connect(server_fd);

    return 0;
}
