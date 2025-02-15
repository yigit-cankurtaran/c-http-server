#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>  // dealing with internet addresses
#include <sys/socket.h> // dealing with sockets, posix. windows needs winsock.h

#define PORT 8080 // on localhost:8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 3

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
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // if socket creation fails (server_fd is negative)
    {
        perror("socket failed");
        return -1;
    }
    printf("Socket created on port %d\n", PORT);
    return server_fd;
}

void bind_socket(int server_fd)
{
    // memset() sets the first n bytes of the memory area pointed to by s to the specified value (0 in this case)
    memset(&address, '0', sizeof(address));
    address.sin_family = AF_INET;         // address family, IPv4
    address.sin_addr.s_addr = INADDR_ANY; // accept connections on all network interfaces
    address.sin_port = htons(PORT);       // convert PORT from host byte order to network byte order
    // we need the conversion because different systems use different conventions for ordering the bytes in a word

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) // if bind fails
    {
        perror("bind failed");
        // BUG: first time bind works properly, running right afterwards i get a "address already in use" error, will fix
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) // we listen to MAX_CLIENTS connections before we reject new ones
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket bound to port %d\n", PORT);
}

int client_connect(int server_fd)
{
    int client_fd;
    char buffer[BUFFER_SIZE] = {0}; // the brackets are used to initialize the array to 0
    const char *message = "Hello, client!\n";

    printf("Waiting for client connection...\n");

    if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        perror("accept");
        return -1;
    }
    printf("Connection accepted on port %d\n", PORT);

    // send message to client
    // use netcat to test: nc -l 8080
    send(client_fd, message, strlen(message), 0); // send client_fd the message, size of message, flags
    printf("Message sent to client\n");

    // read client response
    // we take whatever we write in then press enter, and it will be sent to the server
    read(client_fd, buffer, BUFFER_SIZE); // read client_fd into buffer, size of buffer
    printf("Client response: %s\n", buffer);

    // close connection
    close(client_fd);
    printf("Connection closed\n");

    return client_fd;
}

int main(int argc, char *argv[])
{
    int server_fd = create_socket();
    bind_socket(server_fd);
    client_connect(server_fd);

    return 0;
}
