#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(void)
{
   fd_set ret;
   ret.

   int listen_fd, conn_fd;
   struct sockaddr_in server_addr;
   char buffer[BUFFER_SIZE];

   listen_fd = socket(AF_INET, SOCK_STREAM, 0);
   if (listen_fd < 0)
   {
      perror("socket");
      exit(EXIT_FAILURE);
   }

   int opt = 1;
   setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   server_addr.sin_port = htons(PORT);

   if (bind(listen_fd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)) < 0)
   {
      perror("bind");
      exit(EXIT_FAILURE);
   }

   if (listen(listen_fd, 5) < 0)
   {
      perror("listen");
      exit(EXIT_FAILURE);
   }

   printf("Server listening on port %d...\n", PORT);

   conn_fd = accept(listen_fd, NULL, NULL);
   if (conn_fd < 0)
   {
      perror("accept");
      exit(EXIT_FAILURE);
   }

   printf("Client connected.\n");

   while (1)
   {
      ssize_t n = read(conn_fd, buffer, sizeof(buffer));

      if (n == 0)
         break;

      if (n < 0)
      {
         perror("read");
         break;
      }

      /*
       * Artificial delay so that the stop-and-wait
       * behavior is easy to observe.
       */
      usleep(100000); // 100 ms

      if (write(conn_fd, buffer, n) != n)
      {
         perror("write");
         break;
      }
   }

   close(conn_fd);
   close(listen_fd);

   return 0;
}