#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "lib.h"

#define BUFSIZE 4096 // max number of bytes we can get at once

/**
 * Struct to hold all three pieces of a URL
 */
typedef struct urlinfo_t
{
    char *hostname;
    char *port;
    char *path;
} urlinfo_t;

/**
 * Tokenize the given URL into hostname, path, and port.
 *
 * url: The input URL to parse.
 *
 * Store hostname, path, and port in a urlinfo_t struct and return the struct.
*/
urlinfo_t *parse_url(char *url)
{
    // copy the input URL so as not to mutate the original
    char *hostname = strdup(url);
    char *port;
    char *path;
    char *parsed_path;

    urlinfo_t *urlinfo = malloc(sizeof(urlinfo_t));

    char *backslash = strchr(hostname, '/');
    path = backslash + 1;
    *backslash = '\0';

    char *colon = strchr(hostname, ':');
    port = colon + 1;
    *colon = '\0';

    urlinfo->hostname = hostname;
    urlinfo->path = path;
    urlinfo->port = port;

    return urlinfo;
}

/**
 * Constructs and sends an HTTP request
 *
 * fd:       The file descriptor of the connection.
 * hostname: The hostname string.
 * port:     The port string.
 * path:     The path string.
 *
 * Return the value from the send() function.
*/
int send_request(int fd, char *hostname, char *port, char *path)
{
    const int max_request_size = 16384;
    char request[max_request_size];
    int rv;

    int req_len = sprintf(request,
                          "GET /%s HTTP/1.1\n"
                          "Host: %s:%s\n"
                          "Connection: close\n\n",
                          path, hostname, port);

    rv = send(fd, request, req_len, 0);

    if (rv < 0)
    {
        perror("send_request");
    }

    return rv;
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    char buf[BUFSIZE];

    if (argc != 2)
    {
        fprintf(stderr, "usage: client HOSTNAME:PORT/PATH\n");
        exit(1);
    }

    urlinfo_t *urlinfo = malloc(sizeof(*urlinfo));
    urlinfo = parse_url(argv[1]);

    sockfd = get_socket(urlinfo->hostname, urlinfo->port);
    send_request(sockfd, urlinfo->hostname, urlinfo->port, urlinfo->path);

    while ((numbytes = recv(sockfd, buf, BUFSIZE - 1, 0)) > 0)
    {
        printf("%s\n", buf);
    }

    free(urlinfo);

    close(sockfd);

    return 0;
}
