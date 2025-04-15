#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>  // dealing with internet addresses
#include <sys/socket.h> // dealing with sockets, posix. windows needs winsock.h
#include <signal.h>
#define PORT 8080 // on localhost:8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 3

struct sockaddr_in address;
socklen_t addrlen = sizeof(address);

struct Server
{
    int fd;
    int port;
    int is_running;
};

int create_socket()
{
    int server_fd;
    int opt = 1;
    // we can't immediately re-run the server because the port is still in a "TIME_WAIT" state
    // i will set the socket options to reuse the address even if it's in this state

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // if socket creation fails (server_fd is negative)
    {
        // fd = file descriptor, number that refers to an open file, socket, or pipe
        // socket() creates an endpoint for communication and returns a file descriptor that refers to that endpoint
        // AF_INET = IPv4 internet protocols
        // SOCK_STREAM = TCP
        // 0 = default protocol for stream sockets
        perror("socket failed");
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        // setsockopt() sets the socket options at the option level specified by the level parameter
        // SO_REUSEADDR = socket options reuse address
        // SO_REUSEPORT = socket options reuse port
        perror("setsockopt failed");
        return -1;
    }

    printf("Socket created on port %d\n", PORT);
    return server_fd;
}

void bind_socket(int server_fd)
{
    // memset() sets the first n bytes of the memory area pointed to by s to the specified value (0 in this case)
    memset(&address, 0, sizeof(address)); // set address to 0, not '0'. garbage values can't interfere with the bind
    address.sin_family = AF_INET;         // address family, IPv4
    address.sin_addr.s_addr = INADDR_ANY; // accept connections on all network interfaces
    address.sin_port = htons(PORT);       // convert PORT from host byte order to network byte order
    // we need the conversion because different systems use different conventions for ordering the bytes in a word

    // the same struct sockaddr thing is here as well
    // maybe we're getting the value of the address and casting it to a sockaddr struct?
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) // if bind fails
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) // we listen to MAX_CLIENTS connections before we reject new ones
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket bound to port %d\n", PORT);
}

void graceful_shutdown(int sig, int server_fd)
{
    printf("Shutting down server with signal %d\n", sig);
    close(server_fd);
    printf("Server closed\n");
    exit(0);
}

int client_connect(int server_fd)
{
    int client_fd; // fd = file descriptor again
    char buffer[BUFFER_SIZE] = {0}; // the brackets are used to initialize the array to 0
    const char *message = "Hello, client!\n";

    printf("Waiting for client connection...\n");

    // TODO: what does the struct sockaddr thing mean here?
    if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        perror("client accept failed");
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
    // graceful_shutdown(0, client_fd);

    return client_fd;
}

void run_server(int port)
{
    struct Server server = {
        .fd = create_socket(),
        .port = port,
        .is_running = 1
    };
    bind_socket(server.fd);
    printf("Server created on port %d\n", port);

    while (server.is_running) // while to keep the server running
    {
        int client_fd = client_connect(server.fd);
        if (client_fd < 0)
        {
            // handle shutdowns
            printf("Client disconnected\n");
            server.is_running = 0;
        }
        else
        {
            printf("Client connected\n");
        }
        close(client_fd); // close this one client's socket
    }

    if (server.fd >= 0)
    {
        graceful_shutdown(0, server.fd);
        printf("shutdown complete\n");
    }
}

int main(int argc, char *argv[])
{
    run_server(PORT);

    return 0;
}
