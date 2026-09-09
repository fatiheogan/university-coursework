#define _POSIX_C_SOURCE 200809L
#include "common.h"

/* Set to 1 when the user presses Ctrl+C */
static volatile sig_atomic_t g_sigint = 0;
static void sigint_handler(int sig) { (void)sig; g_sigint = 1; }

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_ip> <tcp_port> <username>\n", argv[0]);
        return 1;
    }
    const char *server_ip = argv[1];
    int tcp_port = parse_int(argv[2]);
    const char *username = argv[3];
    if (tcp_port < 1) { fprintf(stderr, "Invalid port\n"); return 1; }

    /* Create the TCP socket */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); return 1; }

    /* Fill in the server address and connect */
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port   = htons((uint16_t)tcp_port);
    if (inet_pton(AF_INET, server_ip, &sa.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address: %s\n", server_ip); close(sockfd); return 1;
    }
    if (connect(sockfd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        perror("connect"); close(sockfd); return 1;
    }

    /* Install the Ctrl+C handler, ignore broken pipe */
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = sigint_handler; sigemptyset(&act.sa_mask); act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);
    signal(SIGPIPE, SIG_IGN);

    printf("[PROFESSOR %s] CONNECTED server=%s:%d\n", username, server_ip, tcp_port);
    fflush(stdout);

    /* Automatically enroll as a professor */
    {
        char enroll[MAX_LINE];
        snprintf(enroll, sizeof(enroll), "ENROLL PROFESSOR %s", username);
        send_line(sockfd, enroll);
    }

    char rbuf[MAX_LINE + 2];
    int  rlen = 0, done = 0, apparate_sent = 0;

    while (!done) {
        /* On Ctrl+C send APPARATE once, then wait for the reply */
        if (g_sigint && !apparate_sent) {
            printf("[PROFESSOR %s] SENT APPARATE\n", username); fflush(stdout);
            send_line(sockfd, "APPARATE");
            apparate_sent = 1;
        }

        /* Watch the socket, and stdin until APPARATE was sent */
        fd_set rset;
        FD_ZERO(&rset);
        FD_SET(sockfd, &rset);
        if (!apparate_sent) FD_SET(STDIN_FILENO, &rset);
        int maxfd = (sockfd > STDIN_FILENO) ? sockfd : STDIN_FILENO;

        int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready < 0) { if (errno == EINTR) continue; break; }

        /* Data from the server */
        if (FD_ISSET(sockfd, &rset)) {
            ssize_t n = read(sockfd, rbuf + rlen, sizeof(rbuf) - 1 - (size_t)rlen);
            if (n <= 0) {
                printf("[PROFESSOR %s] DISCONNECTED reason=shutdown\n", username); fflush(stdout); done = 1; break;
            }
            rlen += (int)n; rbuf[rlen] = '\0';
            /* Process every complete line in the buffer */
            char *start = rbuf, *nl;
            while ((nl = memchr(start, '\n', (size_t)(rbuf + rlen - start)))) {
                *nl = '\0'; trim_crlf(start);
                printf("[PROFESSOR %s] RECEIVED %s\n", username, start); fflush(stdout);
                /* React to terminating server messages */
                if      (!strcmp(start, "OK APPARATE"))        { printf("[PROFESSOR %s] DISCONNECTED reason=APPARATE\n",  username); fflush(stdout); done = 1; }
                else if (!strcmp(start, "TIMEOUT DISCONNECT"))  { printf("[PROFESSOR %s] DISCONNECTED reason=timeout\n",   username); fflush(stdout); done = 1; }
                else if (!strcmp(start, "SERVER SHUTDOWN"))     { printf("[PROFESSOR %s] DISCONNECTED reason=shutdown\n",  username); fflush(stdout); done = 1; }
                else if (!strncmp(start,"ERR HOGWARTS_FULL",17)){ printf("[PROFESSOR %s] DISCONNECTED reason=HOGWARTS_FULL\n", username); fflush(stdout); done = 1; }
                start = nl + 1;
            }
            /* Keep any leftover partial line for next read */
            int rem = (int)(rbuf + rlen - start);
            if (rem > 0 && start != rbuf) memmove(rbuf, start, (size_t)rem);
            rlen = rem; rbuf[rlen] = '\0';
        }
        if (done) break;

        /* Input typed by the user */
        if (!apparate_sent && FD_ISSET(STDIN_FILENO, &rset)) {
            char ibuf[MAX_LINE + 2];
            if (!fgets(ibuf, sizeof(ibuf), stdin)) {
                /* EOF on stdin: leave gracefully */
                printf("[PROFESSOR %s] SENT APPARATE\n", username); fflush(stdout);
                send_line(sockfd, "APPARATE"); apparate_sent = 1; continue;
            }
            trim_crlf(ibuf);
            if (!ibuf[0]) continue;
            printf("[PROFESSOR %s] SENT %s\n", username, ibuf); fflush(stdout);
            send_line(sockfd, ibuf);
            if (!strcmp(ibuf, "APPARATE")) apparate_sent = 1;
        }
    }
    close(sockfd);
    return 0;
}
