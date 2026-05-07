#define _WIN32_WINNT 0x0600
#include "../include/commun.h"
#include <time.h>

int sock_fd;
int id_bras;
int running = 1;
int outils_obtenus = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void log_message(const char* message) {
    time_t now;
    char timestamp[64];
    time(&now);
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", localtime(&now));
    printf("[%s] [Bras %d] %s\n", timestamp, id_bras, message);
}

void* thread_reflexion(void* arg) {
    while(running) {
        log_message("En reflexion...");
        Sleep(3000);
    }
    return NULL;
}

void* thread_communication(void* arg) {
    char msg[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    
    while(running) {
        snprintf(msg, BUFFER_SIZE, "DEMANDE 0");
        send(sock_fd, msg, strlen(msg), 0);
        
        memset(response, 0, BUFFER_SIZE);
        recv(sock_fd, response, BUFFER_SIZE - 1, 0);
        
        if(strcmp(response, "OK") == 0) {
            log_message("Outil 0 obtenu!");
            pthread_mutex_lock(&mutex);
            outils_obtenus++;
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
        } else {
            log_message("Attente outil 0...");
            Sleep(1000);
            continue;
        }
        
        snprintf(msg, BUFFER_SIZE, "DEMANDE 1");
        send(sock_fd, msg, strlen(msg), 0);
        
        memset(response, 0, BUFFER_SIZE);
        recv(sock_fd, response, BUFFER_SIZE - 1, 0);
        
        if(strcmp(response, "OK") == 0) {
            log_message("Outil 1 obtenu!");
            pthread_mutex_lock(&mutex);
            outils_obtenus++;
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
        } else {
            log_message("Attente outil 1...");
            Sleep(1000);
        }
        
        Sleep(100);
    }
    return NULL;
}

void* thread_assemblage(void* arg) {
    while(running) {
        pthread_mutex_lock(&mutex);
        while(outils_obtenus < 2 && running) {
            pthread_cond_wait(&cond, &mutex);
        }
        pthread_mutex_unlock(&mutex);
        
        if(!running) break;
        
        log_message(">>> ASSEMBLAGE <<<");
        Sleep(3000);
        
        send(sock_fd, "LIBERATION 1", 12, 0);
        send(sock_fd, "LIBERATION 0", 12, 0);
        log_message("Libere outil 1 et 0");
        
        pthread_mutex_lock(&mutex);
        outils_obtenus = 0;
        pthread_mutex_unlock(&mutex);
        
        log_message("=== FIN CYCLE ===\n");
        Sleep(1000);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    pthread_t reflexion_th, comm_th, assemblage_th;
    
    if(argc < 2) {
        printf("Usage: %s <id_bras>\n", argv[0]);
        return 1;
    }
    
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    id_bras = atoi(argv[1]);
    
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if(connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("Erreur connexion\n");
        return 1;
    }
    
    log_message("Connecte!");
    
    pthread_create(&reflexion_th, NULL, thread_reflexion, NULL);
    pthread_create(&comm_th, NULL, thread_communication, NULL);
    pthread_create(&assemblage_th, NULL, thread_assemblage, NULL);
    
    pthread_join(reflexion_th, NULL);
    pthread_join(comm_th, NULL);
    pthread_join(assemblage_th, NULL);
    
    closesocket(sock_fd);
    WSACleanup();
    return 0;
}
