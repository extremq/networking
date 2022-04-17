#include "helpers.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>

int main(int argc, char** argv)
{
    struct sockaddr_in address = create_address();
    int main_socket = init_main_socket(&address);
    
    process_requests(main_socket, &address);
}
