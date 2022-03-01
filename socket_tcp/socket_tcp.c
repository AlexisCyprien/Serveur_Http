#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "socket_tcp.h"

#include <stdio.h>
#include <stdlib.h>

int initSocketTCP(SocketTCP *psocket) {
    if (psocket == NULL) {
        return -1;
    }

    psocket->sockfd = -1;
    psocket->local = NULL;
    psocket->distant = NULL;
    psocket->connected = false;
    psocket->listening = false;
    psocket->bound = false;

    return 0;
}

int connectSocketTCP(SocketTCP *osocket, const char *adresse, u_int16_t port) {
    if (osocket == NULL || adresse == NULL || port == 0) {
        return -1;
    }
    int fd = socket(AF_UNSPEC, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return -1;
    }
    osocket->sockfd = fd;
    adresse_internet *pai = adresse_internet_new(adresse, port);
    if (pai == NULL) {
        return -1;
    }
    osocket->distant = pai;
    osocket->connected = true;

    return 0;
}

int creerSocketEcouteTCP(SocketTCP *isocket, const char *adresse,
                         u_int16_t port) {
    if (isocket == NULL || adresse == NULL || port == 0) {
        return -1;
    }
    int fd = socket(AF_UNSPEC, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return -1;
    }
    isocket->sockfd = fd;
    adresse_internet *pai = adresse_internet_new(adresse, port);
    if (pai == NULL) {
        return -1;
    }
    isocket->local = pai;
    if (listen(isocket->sockfd, SIZE_QUEUE) != 0) {
        perror("listen");
        return -1;
    }
    isocket->listening = true;

    return 0;
}

int acceptSocketTCP(const SocketTCP *secoute, SocketTCP *sservice) {
    if (secoute == NULL) {
        return -1;
    }
    initSocketTCP(sservice);
    struct sockaddr_storage so;
    socklen_t len = sizeof so;
    int fd = accept(secoute->sockfd, (struct sockaddr *)&so, &len);
    if (fd == -1) {
        return -1;
    }
    sservice->sockfd = fd;
    if (so.ss_family == AF_INET6) {
        struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)&so;
        char ip6[INET6_ADDRSTRLEN];
        if (inet_ntop(AF_INET6, &(sa6->sin6_addr), ip6, INET6_ADDRSTRLEN) ==
            NULL) {
            perror("inet_ntop");
            return -1;
        }
        adresse_internet *pai = adresse_internet_new(ip6, sa6->sin6_port);
        if (pai == NULL) {
            return -1;
        }
        sservice->distant = pai;
    } else {
        struct sockaddr_in *sa = (struct sockaddr_in *)&so;
        char ip[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &(sa->sin_addr), ip, INET_ADDRSTRLEN) == NULL) {
            return -1;
        }
        adresse_internet *pai = adresse_internet_new(ip, sa->sin_port);
        if (pai == NULL) {
            perror("inet_ntop");
            return -1;
        }
        sservice->distant = pai;
    }
    sservice->connected = true;
    return 0;
}

int writeSocketTCP(const SocketTCP *osocket, const void *buffer,
                   size_t length) {
    if (osocket == NULL) {
        return -1;
    }

    return send(osocket->sockfd, buffer, length, 0);
}

int readSocketTCP(const SocketTCP *nsocket, void *buffer, size_t length) {
    if (nsocket == NULL) {
        return -1;
    }
    return recv(nsocket->sockfd, buffer, length, 0);
}

int closeSocketTCP(SocketTCP *socket) {
    if (socket == NULL) {
        return -1;
    }
    adresse_internet_free(socket->local);
    adresse_internet_free(socket->distant);
    free(socket);
    return 0;
}