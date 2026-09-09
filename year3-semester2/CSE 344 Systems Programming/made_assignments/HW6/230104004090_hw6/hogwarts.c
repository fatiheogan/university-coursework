#define _POSIX_C_SOURCE 200809L
#include <stdarg.h>
#include "common.h"

/* One spellbook entry for a wizard */
typedef struct {
    char name[MAX_INGREDIENT_NAME];
    int  quantity;
} spellbook_entry_t;

/* State kept for every connected client */
typedef struct {
    int    fd;
    char   username[MAX_USERNAME];
    char   type[16];              /* "WIZARD" or "PROFESSOR" */
    char   line_buf[MAX_LINE + 2];
    int    line_len;
    int    enrolled;
    time_t last_active;
    spellbook_entry_t spellbook[MAX_INGREDIENTS];
    int    spellbook_count;
} client_t;

/* Global server state */
static ingredient_t  g_ingr[MAX_INGREDIENTS];
static int           g_ingr_count  = 0;
static client_t     *g_clients[MAX_CLIENTS_HARD];
static int           g_cli_count   = 0;
static int           g_max_clients = 0;
static int           g_timeout     = 0;
static FILE         *g_logfp       = NULL;
static int           g_listenfd    = -1;
static volatile sig_atomic_t g_shutdown = 0;

/* Print a timestamped event to stdout and the log file */
static void log_event(const char *fmt, ...)
{
    char ts[32];
    get_timestamp(ts, sizeof(ts));
    va_list ap;
    va_start(ap, fmt);
    printf("%s ", ts); vprintf(fmt, ap); printf("\n");
    va_end(ap);
    fflush(stdout);
    if (g_logfp) {
        va_start(ap, fmt);
        fprintf(g_logfp, "%s ", ts); vfprintf(g_logfp, fmt, ap); fprintf(g_logfp, "\n");
        va_end(ap);
        fflush(g_logfp);
    }
}

/* Only set a flag; the main loop does the real work */
static void sigint_handler(int sig) { (void)sig; g_shutdown = 1; }

/* Return the index of an ingredient, or -1 if not found */
static int find_ingr(const char *name)
{
    int i;
    for (i = 0; i < g_ingr_count; i++)
        if (strcmp(g_ingr[i].name, name) == 0) return i;
    return -1;
}

/* Read ingredients.txt; skip lines with bad format */
static int load_ingredients(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) { perror("fopen ingredients"); return -1; }
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        trim_crlf(line);
        if (!line[0]) continue;
        char name[MAX_INGREDIENT_NAME + 1];
        int  qty = 0;
        if (sscanf(line, "%16s %d", name, &qty) != 2 || qty <= 0) continue;
        /* Name must be uppercase letters, digits or underscore */
        int valid = 1, k;
        for (k = 0; name[k]; k++) {
            unsigned char ch = (unsigned char)name[k];
            if (!isupper(ch) && !isdigit(ch) && name[k] != '_') { valid = 0; break; }
        }
        if (!valid || g_ingr_count >= MAX_INGREDIENTS) continue;
        strncpy(g_ingr[g_ingr_count].name, name, MAX_INGREDIENT_NAME - 1);
        g_ingr[g_ingr_count].name[MAX_INGREDIENT_NAME - 1] = '\0';
        g_ingr[g_ingr_count].quantity = qty;
        g_ingr_count++;
    }
    fclose(f);
    return g_ingr_count;
}

/* Find an ingredient in a wizard's spellbook */
static int sb_find(client_t *c, const char *name)
{
    int i;
    for (i = 0; i < c->spellbook_count; i++)
        if (strcmp(c->spellbook[i].name, name) == 0) return i;
    return -1;
}

/* Add quantity to the spellbook (create entry if needed) */
static void sb_add(client_t *c, const char *name, int qty)
{
    int idx = sb_find(c, name);
    if (idx >= 0) { c->spellbook[idx].quantity += qty; return; }
    if (c->spellbook_count >= MAX_INGREDIENTS) return;
    strncpy(c->spellbook[c->spellbook_count].name, name, MAX_INGREDIENT_NAME - 1);
    c->spellbook[c->spellbook_count].name[MAX_INGREDIENT_NAME - 1] = '\0';
    c->spellbook[c->spellbook_count].quantity = qty;
    c->spellbook_count++;
}

/* Find a free slot in the client table */
static int find_slot(void)
{
    int i;
    for (i = 0; i < MAX_CLIENTS_HARD; i++) if (!g_clients[i]) return i;
    return -1;
}

/* Check if a username is already in use */
static int username_taken(const char *name)
{
    int i;
    for (i = 0; i < MAX_CLIENTS_HARD; i++)
        if (g_clients[i] && g_clients[i]->enrolled &&
            strcmp(g_clients[i]->username, name) == 0) return 1;
    return 0;
}

/* Close the socket, free state and log the disconnect */
static void drop_client(int idx, const char *reason)
{
    client_t *c = g_clients[idx];
    if (!c) return;
    log_event("CLIENT_DISCONNECTED username=%s reason=%s",
              c->enrolled ? c->username : "(unenrolled)", reason);
    close(c->fd);
    free(c);
    g_clients[idx] = NULL;
    g_cli_count--;
}

/* Handle ENROLL: validate type and unique username */
static void cmd_enroll(client_t *c, char *args)
{
    char type[16], uname[MAX_USERNAME];
    if (sscanf(args, "%15s %31s", type, uname) != 2 ||
        (strcmp(type,"WIZARD") != 0 && strcmp(type,"PROFESSOR") != 0)) {
        send_line(c->fd, "ERR UNKNOWN ENROLL"); return;
    }
    if (username_taken(uname)) { send_line(c->fd, "ERR ENROLL name_taken"); return; }
    strncpy(c->username, uname, MAX_USERNAME - 1); c->username[MAX_USERNAME-1] = '\0';
    strncpy(c->type, type, 15);                    c->type[15] = '\0';
    c->enrolled = 1;
    char resp[MAX_LINE];
    snprintf(resp, sizeof(resp), "OK ENROLL %s", uname);
    send_line(c->fd, resp);
    log_event("ENROLL username=%s type=%s fd=%d", uname, type, c->fd);
}

/* Handle BREW: add to global stock and spellbook */
static void cmd_brew(client_t *c, char *args)
{
    char name[MAX_INGREDIENT_NAME + 1]; int qty = 0;
    if (sscanf(args, "%16s %d", name, &qty) != 2 || qty <= 0) {
        send_line(c->fd, "ERR UNKNOWN_INGREDIENT"); return;
    }
    int idx = find_ingr(name);
    if (idx < 0) { send_line(c->fd, "ERR UNKNOWN_INGREDIENT"); return; }
    int old = g_ingr[idx].quantity;
    g_ingr[idx].quantity += qty;
    sb_add(c, name, qty);
    char resp[MAX_LINE];
    snprintf(resp, sizeof(resp), "OK BREW %s %d %d", name, qty, g_ingr[idx].quantity);
    send_line(c->fd, resp);
    log_event("BREW wizard=%s ingredient=%s qty=%d old_qty=%d new_qty=%d",
              c->username, name, qty, old, g_ingr[idx].quantity);
}

/* Handle CONSUME: remove from global stock and spellbook */
static void cmd_consume(client_t *c, char *args)
{
    char name[MAX_INGREDIENT_NAME + 1]; int qty = 0;
    if (sscanf(args, "%16s %d", name, &qty) != 2 || qty <= 0) {
        send_line(c->fd, "ERR UNKNOWN_INGREDIENT"); return;
    }
    int idx = find_ingr(name);
    if (idx < 0) { send_line(c->fd, "ERR UNKNOWN_INGREDIENT"); return; }
    int sidx = sb_find(c, name);
    int have = (sidx >= 0) ? c->spellbook[sidx].quantity : 0;
    /* Need enough in both global stock and the wizard's spellbook */
    if (g_ingr[idx].quantity < qty || have < qty) {
        send_line(c->fd, "ERR INSUFFICIENT_INGREDIENTS"); return;
    }
    int old = g_ingr[idx].quantity;
    g_ingr[idx].quantity       -= qty;
    c->spellbook[sidx].quantity -= qty;
    char resp[MAX_LINE];
    snprintf(resp, sizeof(resp), "OK CONSUME %s %d %d", name, qty, g_ingr[idx].quantity);
    send_line(c->fd, resp);
    log_event("CONSUME wizard=%s ingredient=%s qty=%d old_qty=%d new_qty=%d",
              c->username, name, qty, old, g_ingr[idx].quantity);
}

/* Handle SPELLBOOK: list the wizard's own inventory */
static void cmd_spellbook(client_t *c)
{
    char resp[MAX_LINE * 4];
    int pos = snprintf(resp, sizeof(resp), "OK SPELLBOOK "), first = 1, i;
    for (i = 0; i < c->spellbook_count; i++) {
        if (c->spellbook[i].quantity <= 0) continue;
        int w = snprintf(resp+pos, sizeof(resp)-(size_t)pos, "%s%s:%d",
                         first?"":","  , c->spellbook[i].name, c->spellbook[i].quantity);
        if (w < 0) { break; } pos += w; first = 0;
    }
    if (first) { send_line(c->fd, "OK SPELLBOOK EMPTY"); return; }
    send_line(c->fd, resp);
}

/* Handle INSPECT: report one ingredient's global quantity */
static void cmd_inspect(client_t *c, char *args)
{
    char name[MAX_INGREDIENT_NAME + 1];
    if (sscanf(args, "%16s", name) != 1) { send_line(c->fd, "ERR UNKNOWN_INGREDIENT"); return; }
    int idx = find_ingr(name);
    if (idx < 0) { send_line(c->fd, "ERR UNKNOWN_INGREDIENT"); return; }
    char resp[MAX_LINE];
    snprintf(resp, sizeof(resp), "OK INSPECT %s %d", name, g_ingr[idx].quantity);
    send_line(c->fd, resp);
    log_event("INSPECT professor=%s ingredient=%s qty=%d", c->username, name, g_ingr[idx].quantity);
}

/* Handle SCROLL: full snapshot of all ingredients */
static void cmd_scroll(client_t *c)
{
    char resp[MAX_LINE * 8];
    int pos = snprintf(resp, sizeof(resp), "OK SCROLL "), i;
    for (i = 0; i < g_ingr_count; i++) {
        int w = snprintf(resp+pos, sizeof(resp)-(size_t)pos, "%s%s:%d",
                         i==0?"":","  , g_ingr[i].name, g_ingr[i].quantity);
        if (w < 0) { break; } pos += w;
    }
    send_line(c->fd, resp);
    log_event("SCROLL professor=%s ingredients=%d", c->username, g_ingr_count);
}

/* Handle ROSTER: list all enrolled usernames */
static void cmd_roster(client_t *c)
{
    char resp[MAX_LINE * 8];
    int pos = snprintf(resp, sizeof(resp), "OK ROSTER "), first = 1, cnt = 0, i;
    for (i = 0; i < MAX_CLIENTS_HARD; i++) {
        if (!g_clients[i] || !g_clients[i]->enrolled) continue;
        int w = snprintf(resp+pos, sizeof(resp)-(size_t)pos, "%s%s",
                         first?"":"," , g_clients[i]->username);
        if (w < 0) { break; } pos += w; first = 0; cnt++;
    }
    send_line(c->fd, resp);
    log_event("ROSTER professor=%s clients=%d", c->username, cnt);
}

/* Parse one command line; return 1 if the client should leave */
static int dispatch(client_t *c, char *line)
{
    trim_crlf(line);
    if (!line[0]) return 0;

    /* Before enrollment only ENROLL is allowed */
    if (!c->enrolled) {
        if (strncmp(line, "ENROLL ", 7) == 0) cmd_enroll(c, line + 7);
        else send_line(c->fd, "ERR NOT_ENROLLED");
        return 0;
    }

    int is_prof = (strcmp(c->type, "PROFESSOR") == 0);
    int is_wiz  = !is_prof;

    /* Route the command and enforce per-type permissions */
    if (strncmp(line, "ENROLL ", 7) == 0) {
        send_line(c->fd, "ERR UNKNOWN ENROLL");
    } else if (strncmp(line, "BREW", 4) == 0 && (line[4]==' '||line[4]=='\0')) {
        if (is_prof) { send_line(c->fd, "ERR UNAUTHORIZED"); return 0; }
        cmd_brew(c, line[4]==' ' ? line+5 : "");
    } else if (strncmp(line, "CONSUME", 7) == 0 && (line[7]==' '||line[7]=='\0')) {
        if (is_prof) { send_line(c->fd, "ERR UNAUTHORIZED"); return 0; }
        cmd_consume(c, line[7]==' ' ? line+8 : "");
    } else if (strcmp(line, "SPELLBOOK") == 0) {
        if (is_prof) { send_line(c->fd, "ERR UNAUTHORIZED"); return 0; }
        cmd_spellbook(c);
    } else if (strncmp(line, "INSPECT", 7) == 0 && (line[7]==' '||line[7]=='\0')) {
        if (is_wiz) { send_line(c->fd, "ERR UNAUTHORIZED"); return 0; }
        cmd_inspect(c, line[7]==' ' ? line+8 : "");
    } else if (strcmp(line, "SCROLL") == 0) {
        if (is_wiz) { send_line(c->fd, "ERR UNAUTHORIZED"); return 0; }
        cmd_scroll(c);
    } else if (strcmp(line, "ROSTER") == 0) {
        if (is_wiz) { send_line(c->fd, "ERR UNAUTHORIZED"); return 0; }
        cmd_roster(c);
    } else if (strcmp(line, "APPARATE") == 0) {
        send_line(c->fd, "OK APPARATE");
        return 1;
    } else {
        char err[MAX_LINE];
        snprintf(err, sizeof(err), "ERR UNKNOWN %s", line);
        send_line(c->fd, err);
    }
    return 0;
}

/* Read available data and process complete lines.
   Returns 0 ok, -1 hangup, -2 client sent APPARATE */
static int read_client(int idx)
{
    client_t *c = g_clients[idx];
    int room = (int)(sizeof(c->line_buf) - 1) - c->line_len;
    /* Buffer is full without a newline: line is too long */
    if (room <= 0) { send_line(c->fd, "ERR TOOLONG"); c->line_len = 0; return 0; }

    ssize_t n = read(c->fd, c->line_buf + c->line_len, (size_t)room);
    if (n < 0) { if (errno == EINTR) return 0; return -1; }
    if (n == 0) return -1;

    c->line_len += (int)n;
    c->line_buf[c->line_len] = '\0';

    /* Handle every full line; partial data stays buffered */
    char *start = c->line_buf, *nl;
    while ((nl = memchr(start, '\n', (size_t)(c->line_buf + c->line_len - start)))) {
        *nl = '\0';
        if ((int)(nl - start) >= MAX_LINE) {
            send_line(c->fd, "ERR TOOLONG");
        } else {
            c->last_active = time(NULL);
            if (dispatch(c, start)) {
                /* APPARATE: keep leftover bytes, then signal exit */
                start = nl + 1;
                int rem = (int)(c->line_buf + c->line_len - start);
                if (rem > 0) memmove(c->line_buf, start, (size_t)rem);
                c->line_len = rem;
                c->line_buf[c->line_len] = '\0';
                return -2;
            }
        }
        start = nl + 1;
    }
    /* Move any incomplete line to the front of the buffer */
    int rem = (int)(c->line_buf + c->line_len - start);
    if (rem > 0 && start != c->line_buf) memmove(c->line_buf, start, (size_t)rem);
    c->line_len = rem;
    c->line_buf[c->line_len] = '\0';
    return 0;
}

/* Notify all clients, close everything and log cleanup */
static void do_shutdown(void)
{
    int i;
    for (i = 0; i < MAX_CLIENTS_HARD; i++) {
        if (!g_clients[i]) continue;
        send_line(g_clients[i]->fd, "SERVER SHUTDOWN");
        close(g_clients[i]->fd);
        free(g_clients[i]);
        g_clients[i] = NULL;
    }
    log_event("SHUTDOWN signal=SIGINT");
    log_event("CLEANUP_DONE clients=0");
    if (g_logfp)    { fclose(g_logfp);    g_logfp    = NULL; }
    if (g_listenfd >= 0) { close(g_listenfd); g_listenfd = -1; }
}

int main(int argc, char *argv[])
{
    int tcp_port = -1, max_clients = -1, timeout_sec = -1;
    const char *ingr_path = NULL, *log_path = NULL;
    int i;

    /* Parse command-line flags */
    for (i = 1; i < argc; i++) {
        if      (!strcmp(argv[i],"-p") && i+1<argc) tcp_port    = parse_int(argv[++i]);
        else if (!strcmp(argv[i],"-s") && i+1<argc) ingr_path   = argv[++i];
        else if (!strcmp(argv[i],"-l") && i+1<argc) log_path    = argv[++i];
        else if (!strcmp(argv[i],"-n") && i+1<argc) max_clients = parse_int(argv[++i]);
        else if (!strcmp(argv[i],"-t") && i+1<argc) timeout_sec = parse_int(argv[++i]);
        else goto usage;
    }
    if (tcp_port < 1024 || !ingr_path || !log_path || max_clients < 1 || timeout_sec < 1)
        goto usage;
    if (max_clients >= MAX_CLIENTS_HARD) max_clients = MAX_CLIENTS_HARD - 1;
    g_max_clients = max_clients;
    g_timeout     = timeout_sec;

    /* Load ingredients and open the log file */
    if (load_ingredients(ingr_path) < 0) {
        fprintf(stderr, "Cannot load ingredients: %s\n", ingr_path); return 1;
    }
    g_logfp = fopen(log_path, "a");
    if (!g_logfp) { perror("fopen logfile"); return 1; }

    /* Create, bind and listen on the TCP socket */
    g_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (g_listenfd < 0) { perror("socket"); return 1; }
    { int opt = 1; setsockopt(g_listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); }
    {
        struct sockaddr_in sa;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family      = AF_INET;
        sa.sin_addr.s_addr = INADDR_ANY;
        sa.sin_port        = htons((uint16_t)tcp_port);
        if (bind(g_listenfd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
            perror("bind"); close(g_listenfd); return 1;
        }
    }
    if (listen(g_listenfd, 128) < 0) { perror("listen"); close(g_listenfd); return 1; }

    /* Catch Ctrl+C, ignore broken pipe */
    {
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = sigint_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGINT, &sa, NULL);
        signal(SIGPIPE, SIG_IGN);
    }

    printf("Hogwarts is ready. Port: %d | Max Clients: %d | Timeout: %ds\n",
           tcp_port, max_clients, timeout_sec);
    fflush(stdout);
    log_event("SERVER_STARTED port=%d max_clients=%d timeout=%d ingredients=%d",
              tcp_port, max_clients, timeout_sec, g_ingr_count);

    while (!g_shutdown) {
        /* 1. Disconnect any client idle longer than the timeout */
        time_t now = time(NULL);
        for (i = 0; i < MAX_CLIENTS_HARD; i++) {
            if (!g_clients[i]) continue;
            double elapsed = difftime(now, g_clients[i]->last_active);
            if (elapsed >= (double)g_timeout) {
                send_line(g_clients[i]->fd, "TIMEOUT DISCONNECT");
                log_event("TIMEOUT username=%s fd=%d elapsed=%ds",
                          g_clients[i]->enrolled ? g_clients[i]->username : "(unenrolled)",
                          g_clients[i]->fd, (int)elapsed);
                drop_client(i, "timeout");
            }
        }
        if (g_shutdown) break;

        /* 2. Build the fd set and the smallest select timeout */
        fd_set rset;
        FD_ZERO(&rset);
        FD_SET(g_listenfd, &rset);
        int maxfd = g_listenfd;
        time_t min_exp = now + (time_t)g_timeout;
        for (i = 0; i < MAX_CLIENTS_HARD; i++) {
            if (!g_clients[i]) continue;
            FD_SET(g_clients[i]->fd, &rset);
            if (g_clients[i]->fd > maxfd) maxfd = g_clients[i]->fd;
            time_t exp = g_clients[i]->last_active + (time_t)g_timeout;
            if (exp < min_exp) min_exp = exp;
        }
        now = time(NULL);
        long wait = (long)difftime(min_exp, now);
        if (wait < 0) wait = 0;
        if (wait > (long)g_timeout) wait = (long)g_timeout;
        struct timeval tv;
        tv.tv_sec  = wait;
        tv.tv_usec = 0;

        int nready = select(maxfd + 1, &rset, NULL, NULL, &tv);
        if (nready < 0) { if (errno == EINTR) continue; perror("select"); break; }
        if (g_shutdown) break;

        /* 3. Accept a new connection (reject if at capacity) */
        if (FD_ISSET(g_listenfd, &rset)) {
            struct sockaddr_in ca;
            socklen_t clen = sizeof(ca);
            int newfd = accept(g_listenfd, (struct sockaddr *)&ca, &clen);
            if (newfd >= 0) {
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &ca.sin_addr, ip, sizeof(ip));
                if (g_cli_count >= g_max_clients) {
                    send_line(newfd, "ERR HOGWARTS_FULL");
                    log_event("REJECTED fd=%d ip=%s reason=HOGWARTS_FULL", newfd, ip);
                    close(newfd);
                } else {
                    int slot = find_slot();
                    if (slot < 0) { send_line(newfd, "ERR HOGWARTS_FULL"); close(newfd); }
                    else {
                        client_t *nc = (client_t *)calloc(1, sizeof(client_t));
                        if (!nc) { close(newfd); }
                        else {
                            nc->fd = newfd; nc->enrolled = 0; nc->line_len = 0;
                            nc->last_active = time(NULL); nc->spellbook_count = 0;
                            g_clients[slot] = nc; g_cli_count++;
                            log_event("CLIENT_CONNECTED fd=%d ip=%s", newfd, ip);
                        }
                    }
                }
            }
        }

        /* 4. Handle data from existing clients */
        for (i = 0; i < MAX_CLIENTS_HARD; i++) {
            if (!g_clients[i] || !FD_ISSET(g_clients[i]->fd, &rset)) continue;
            int ret = read_client(i);
            if      (ret == -2) drop_client(i, "APPARATE");
            else if (ret == -1) drop_client(i, "hangup");
        }
    }

    do_shutdown();
    return 0;

usage:
    fprintf(stderr,
        "Usage: %s -p <port(>=1024)> -s <ingredients.txt> -l <logfile>"
        " -n <max_clients(>=1)> -t <timeout(>=1)>\n", argv[0]);
    return 1;
}
