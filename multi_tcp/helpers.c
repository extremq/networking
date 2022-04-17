#include "helpers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>    
#include <errno.h>     
#include <unistd.h>    
#include <arpa/inet.h> 
#include <sys/types.h> 
#include <netinet/in.h>
#include <sys/time.h>  

struct sockaddr_in create_address()
{
    struct sockaddr_in address;
    // Set type of socket
    address.sin_family = AF_INET; // IPv4 protocol
    address.sin_addr.s_addr = INADDR_ANY; // Accept any incoming address
    address.sin_port = htons(PORT); // Transform the port into network-byte order

    return address;
}

int init_main_socket(struct sockaddr_in* address) 
{
    int option = TRUE;
    // Create a server socket
    int main_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (0 > main_socket)
    {
        perror("Socket failed.");
        exit(EXIT_FAILURE);
    }

    // Allow server socket to have multiple connections
    if (0 > setsockopt(main_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&option, sizeof(option)))
    {
        perror("Setting socket options failed.");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to a specific port
    if (0 > bind(main_socket, (struct sockaddr *)address, sizeof(*address)))
    {
        perror("Binding failed.");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Listener active on port %d.\n", PORT);

    if (0 > listen(main_socket, MAX_PENDING_CONNECTIONS))
    {
        perror("Listen function fail.\n");
        exit(EXIT_FAILURE);
    }

    return main_socket;
}

void process_requests(int main_socket, struct sockaddr_in* address)
{
    char buffer[2049] = {0}, temp_buffer[4097] = {0};

    int addrlen = sizeof(address);
    int max_fd, i, j, fd, temp_fd, activity, new_socket, val_read, length;
    int client_socket[MAX_CLIENTS] = {0}; // Client array

    char* message = "You are now connected to the server!\n";

    // Set of file descriptors
    fd_set readfds;
    for(;;)
    {
        // Reset the set
        FD_ZERO(&readfds);

        FD_SET(main_socket, &readfds);
        max_fd = main_socket;

        for(i = 0; i < MAX_CLIENTS; ++i)
        {
            fd = client_socket[i];

            // Check if socket is valid
            if(fd > 0)
                FD_SET(fd, &readfds);

            // Need the max fd for select
            if(fd > max_fd)
                max_fd = fd;
        }

        activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            perror("Select error.");
        }

        if (FD_ISSET(main_socket, &readfds))
        {
            if(0 > (new_socket = accept(main_socket,
                            (struct sockaddr *)address, (socklen_t*)&addrlen)))
            {
                perror("Accept failure.");
                exit(EXIT_FAILURE);
            }

            fprintf(stdout, "New connection, socket fd is %d, ip is: %s, port: %d\n",
                    new_socket, inet_ntoa(address->sin_addr), ntohs
                    (address->sin_port));

            if(send(new_socket, message, strlen(message), 0) != strlen(message))
            {
                perror("Send failure.");
            }

            fprintf(stdout, "Welcome sent.\n");

            for(i = 0; i < MAX_CLIENTS; ++i)
            {
                if(client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    fprintf(stdout, "Added to list of sockets as %d\n", i);

                    break;
                }
            }
        }

        for(i = 0; i < MAX_CLIENTS; ++i)
        {
            fd = client_socket[i];

            if (FD_ISSET(fd, &readfds))
            {
                getpeername(fd, (struct sockaddr*)address, (socklen_t*)&addrlen);
                if ((val_read = read(fd, buffer, 2048)) == 0)
                {
                    fprintf(stdout, "Disconnected socket with ip %s, port %d.\n",
                            inet_ntoa(address->sin_addr), ntohs(address->sin_port));

                    close(fd);
                    client_socket[i] = 0;
                }
                else
                {
                    buffer[val_read] = '\0';
                    length = snprintf(NULL, 0, "User %d: %s\n", i, buffer);
                    snprintf(temp_buffer, length ,"User %d: %s\n", i, buffer);
                    
                    for(j = 0; j < MAX_CLIENTS; ++j)
                    {
                        temp_fd = client_socket[j];
                        if (i != j && temp_fd > 0)
                        {
                            send(temp_fd, temp_buffer, strlen(temp_buffer), 0);
                        }
                    }
                }
            }
        }
    }
}
