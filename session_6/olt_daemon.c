#define _GNU_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9000
#define BACKLOG 10
#define MAX_ONU 16

typedef struct
{
    int id;
    char name[32];
    int online;
    unsigned long alarm_count;
} ONU;

typedef struct
{
    int fd;
    int client_id;
} Client;

/* ============================================================
 * Global OLT state
 * ============================================================ */

static ONU *onu_table[MAX_ONU];

static pthread_mutex_t table_mutex =
    PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t alarm_mutex =
    PTHREAD_MUTEX_INITIALIZER;

static volatile sig_atomic_t running = 1;

/* ============================================================
 * Signal handler
 * ============================================================ */

static void handle_sigint(int sig)
{
    (void)sig;

    running = 0;
}

/* ============================================================
 * Logging
 * ============================================================ */

static void log_message(const char *message)
{
    printf("[OLT] %s\n", message);
    fflush(stdout);
}

/* ============================================================
 * Find ONU
 * ============================================================ */

static ONU *find_onu(int id)
{
    for (int i = 0; i < MAX_ONU; i++)
    {
        if (onu_table[i] != NULL &&
            onu_table[i]->id == id)
        {
            return onu_table[i];
        }
    }

    return NULL;
}

/* ============================================================
 * Initialize ONU table
 * ============================================================ */

static void init_onus(void)
{
    for (int i = 0; i < MAX_ONU; i++)
    {
        onu_table[i] = NULL;
    }

    for (int i = 0; i < 8; i++)
    {
        onu_table[i] = malloc(sizeof(ONU));

        if (onu_table[i] == NULL)
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        onu_table[i]->id = i;

        snprintf(
            onu_table[i]->name,
            sizeof(onu_table[i]->name),
            "ONU-%02d",
            i);

        onu_table[i]->online =
            (i % 2 == 0);

        onu_table[i]->alarm_count = 0;
    }
}

/* ============================================================
 * SHOW ONU
 * ============================================================ */

static void show_onus(int fd)
{
    char response[1024];

    pthread_mutex_lock(&table_mutex);

    int offset = 0;

    offset += snprintf(
        response + offset,
        sizeof(response) - offset,
        "========== ONU TABLE ==========\n");

    for (int i = 0; i < MAX_ONU; i++)
    {
        if (onu_table[i] != NULL)
        {
            offset += snprintf(
                response + offset,
                sizeof(response) - offset,

                "ONU %d | %-8s | %-4s | alarms=%lu\n",

                onu_table[i]->id,
                onu_table[i]->name,

                onu_table[i]->online
                    ? "UP"
                    : "DOWN",

                onu_table[i]->alarm_count);
        }
    }

    pthread_mutex_unlock(&table_mutex);

    send(
        fd,
        response,
        strlen(response),
        0);
}

/* ============================================================
 * SET ONU STATE
 * ============================================================ */

static void set_onu_state(
    int id,
    int state)
{
    pthread_mutex_lock(&table_mutex);

    ONU *onu = find_onu(id);

    if (onu != NULL)
    {
        onu->online = state;
    }

    pthread_mutex_unlock(&table_mutex);
}

/* ============================================================
 * Generate alarm
 *
 * Lock order:
 *
 *      alarm_mutex
 *          ↓
 *      table_mutex
 *
 * ============================================================ */

static void generate_alarm(int id)
{
    pthread_mutex_lock(&alarm_mutex);

    /*
     * Artificial delay.
     *
     * This makes the race/deadlock easier to reproduce.
     */
    usleep(100000);

    pthread_mutex_lock(&table_mutex);

    ONU *onu = find_onu(id);

    if (onu != NULL)
    {
        onu->alarm_count++;
    }

    pthread_mutex_unlock(&table_mutex);

    pthread_mutex_unlock(&alarm_mutex);
}

/* ============================================================
 * Config + Alarm
 *
 * BUG:
 *
 * This function uses the opposite lock order:
 *
 *      table_mutex
 *          ↓
 *      alarm_mutex
 *
 * generate_alarm():
 *
 *      alarm_mutex
 *          ↓
 *      table_mutex
 *
 * Two threads can therefore deadlock.
 * ============================================================ */

static void config_alarm(int id)
{
    pthread_mutex_lock(&table_mutex);

    ONU *onu = find_onu(id);

    if (onu != NULL)
    {
        onu->online =
            !onu->online;
    }

    usleep(100000);

    pthread_mutex_lock(&alarm_mutex);

    if (onu != NULL)
    {
        onu->alarm_count++;
    }

    pthread_mutex_unlock(&alarm_mutex);

    pthread_mutex_unlock(&table_mutex);
}

/* ============================================================
 * CLIENT THREAD
 * ============================================================ */

static void *client_worker(void *arg)
{
    Client *client = (Client *)arg;

    int fd = client->fd;
    int client_id = client->client_id;

    free(client);

    char buffer[256];

    printf(
        "[THREAD] client=%d fd=%d started\n",
        client_id,
        fd);

    fflush(stdout);

    while (running)
    {
        memset(buffer, 0, sizeof(buffer));

        ssize_t n =
            recv(
                fd,
                buffer,
                sizeof(buffer) - 1,
                0);

        if (n == 0)
        {
            printf(
                "[THREAD] client=%d disconnected\n",
                client_id);

            break;
        }

        if (n < 0)
        {
            if (errno == EINTR)
                continue;

            perror("recv");
            break;
        }

        buffer[n] = '\0';

        printf( "[THREAD %d] command: %s",
            client_id,
            buffer);

        fflush(stdout);

        /* ----------------------------------------------------
         * SHOW
         * ---------------------------------------------------- */

        if (strncmp(buffer, "show", 4) == 0)
        {
            show_onus(fd);
        }

        /* ----------------------------------------------------
         * ONU <id>
         *
         * BUG:
         *
         * No range check on id.
         *
         * "onu 99"
         *
         * accesses:
         *
         *      onu_table[99]
         *
         * ---------------------------------------------------- */

        else if (sscanf(buffer, "onu %d", &client_id) == 1)
        {
            pthread_mutex_lock(&table_mutex);

            ONU *onu = onu_table[client_id];

            char response[256];

            /*
             * If onu == NULL, dereferencing it causes crash.
             *
             * If id is outside the array, behavior is undefined.
             */

            snprintf(response, sizeof(response), "ONU %d\n"
                                                 "name   : %s\n"
                                                 "state  : %s\n"
                                                 "alarms : %lu\n",
                     onu->id,
                     onu->name,

                     onu->online
                         ? "UP"
                         : "DOWN",

                     onu->alarm_count);

            pthread_mutex_unlock(&table_mutex);

            send(fd, response, strlen(response), 0);
        }

        /* ----------------------------------------------------
         * SET UP
         * ---------------------------------------------------- */

        else if (sscanf(buffer, "set %d up", &client_id) == 1)
        {
            set_onu_state(client_id, 1);

            send(fd, "OK\n", 3, 0);
        }

        /* ----------------------------------------------------
         * SET DOWN
         * ---------------------------------------------------- */

        else if (sscanf(buffer, "set %d down", &client_id) == 1)
        {
            set_onu_state(client_id, 0);

            send(fd, "OK\n", 3, 0);
        }

        /* ----------------------------------------------------
         * ALARM
         * ---------------------------------------------------- */

        else if (sscanf(buffer, "alarm %d", &client_id) == 1)
        {
            generate_alarm(client_id);

            send(fd, "ALARM OK\n", 9, 0);
        }

        /* ----------------------------------------------------
         * CONFIG-ALARM
         *
         * This is the command used to trigger deadlock.
         * ---------------------------------------------------- */

        else if (sscanf(buffer, "config-alarm %d", &client_id) == 1)
        {
            config_alarm(client_id);

            send(fd, "CONFIG OK\n", 10, 0);
        }

        /* ----------------------------------------------------
         * QUIT
         * ---------------------------------------------------- */

        else if (strncmp(buffer, "quit", 4) == 0)
        {
            break;
        }

        /* ----------------------------------------------------
         * UNKNOWN COMMAND
         * ---------------------------------------------------- */

        else
        {
            send(fd, "ERR unknown command\n", 20, 0);
        }
    }

    close(fd);

    printf("[THREAD] client=%d stopped\n", client_id);

    fflush(stdout);

    return NULL;
}

/* ============================================================
 * MAIN
 * ============================================================ */

int main(void)
{
    signal(SIGINT, handle_sigint);

    /*
     * Prevent client disconnect from killing
     * the whole daemon through SIGPIPE.
     */
    signal(SIGPIPE, SIG_IGN);

    init_onus();

    /* --------------------------------------------------------
     * socket()
     * -------------------------------------------------------- */

    int server_fd = socket(AF_INET,
                           SOCK_STREAM,
                           0);

    if (server_fd < 0)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    int yes = 1;

    setsockopt(server_fd,
               SOL_SOCKET,
               SO_REUSEADDR,
               &yes,
               sizeof(yes));

    /* --------------------------------------------------------
     * bind()
     * -------------------------------------------------------- */

    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;

    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(server_fd);
        return EXIT_FAILURE;
    }

    /* --------------------------------------------------------
     * listen()
     * -------------------------------------------------------- */

    if (listen(server_fd, BACKLOG) < 0)
    {
        perror("listen");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("[OLT] Management daemon listening on TCP %d\n", PORT);

    fflush(stdout);

    int next_client_id = 1;

    /* --------------------------------------------------------
     * ACCEPT LOOP
     * -------------------------------------------------------- */

    while (running)
    {
        struct sockaddr_in client_addr;

        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd,
                               (struct sockaddr *)&client_addr,
                               &client_len);

        if (client_fd < 0)
        {
            if (errno == EINTR)
                continue;

            perror("accept");
            break;
        }

        Client *client = malloc(sizeof(Client));

        if (client == NULL)
        {
            perror("malloc");
            close(client_fd);
            continue;
        }

        client->fd = client_fd;

        client->client_id = next_client_id++;

        pthread_t thread;

        int ret = pthread_create(&thread,
                                 NULL,
                                 client_worker,
                                 client);

        if (ret != 0)
        {
            fprintf(stderr,
                    "pthread_create: %s\n",
                    strerror(ret));

            close(client_fd);
            free(client);
            continue;
        }

        /*
         * We intentionally detach the worker.
         *
         * This makes debugging the thread lifecycle
         * more realistic.
         */
        pthread_detach(thread);
    }

    close(server_fd);

    /* --------------------------------------------------------
     * Cleanup
     * -------------------------------------------------------- */

    for (int i = 0; i < MAX_ONU; i++)
    {
        free(onu_table[i]);
    }

    log_message("OLT daemon stopped");

    return 0;
}