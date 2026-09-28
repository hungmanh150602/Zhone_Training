#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5555

int main(void)
{
    int server_fd;
    int client_fd;

    struct sockaddr_in addr;

    /*
     * Step 1: Try to open a log file.
     */
    FILE *log = fopen("/root/strace_lab.log", "a");

    if (log != NULL)
    {
        fprintf(log, "Server starting...\n");
        fclose(log);
    }

    /*
     * Step 2: Create TCP socket.
     */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        return 1;
    }

    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    /*
     * Step 3: Bind.
     */
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        close(server_fd);
        return 1;
    }

    /*
     * Step 4: Listen.
     */
    if (listen(server_fd, 5) < 0)
    {
        close(server_fd);
        return 1;
    }

    /*
     * Deliberately no printf().
     * The program appears to "do nothing".
     */

    while (1)
    {
        /*
         * Program hangs here waiting for client.
         */
        client_fd = accept(server_fd, NULL, NULL);

        if (client_fd < 0)
        {
            continue;
        }

        char buffer[128];

        /*
         * Another possible blocking point.
         */
        ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);

        if (n > 0)
        {
            buffer[n] = '\0';
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}