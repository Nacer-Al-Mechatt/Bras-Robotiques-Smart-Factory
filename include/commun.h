#ifndef COMMUN_H
#define COMMUN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
#endif

#define PORT 8888
#define MAX_CLIENTS 10
#define MAX_OUTILS 5
#define BUFFER_SIZE 256

#define OUTIL_LIBRE 0
#define OUTIL_OCCUPE 1

#define MSG_DEMANDE 1
#define MSG_LIBERATION 2
#define MSG_OK 3
#define MSG_WAIT 4
#define MSG_DEADLOCK 5

typedef struct {
    int id;
    int etat;
    int possesseur_id;
    pthread_mutex_t mutex;
} Outil;

typedef struct {
    int id;
    int socket_fd;
    pthread_t thread_communication;
    pthread_t thread_idle;
    pthread_t thread_execution;
    int outils_obtenus[2];
    int nb_outils;
    time_t timestamp;
} BrasRobotique;

#endif