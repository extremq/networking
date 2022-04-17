#ifndef HELPERS_H
#define HELPERS_H

#define TRUE  1
#define FALSE 0
#define PORT  8888
#define MAX_PENDING_CONNECTIONS 5
#define MAX_CLIENTS 32

struct sockaddr_in create_address();
int init_main_socket(struct sockaddr_in* address); 
void process_requests(int main_socket, struct sockaddr_in* address);

#endif
