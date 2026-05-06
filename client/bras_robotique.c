#define _WIN32_WINNT 0x0600
#include "../include/commun.h"
#include <time.h>

int sock_fd;
int id_bras;
int running = 1;

void log_message(const char* message) {
    time_t now;
    char timestamp[64];
    time(&now);
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", localtime(&now));
    printf("[%s] [Bras %d] %s\n", timestamp, id_bras, message);
}

void obtenir_outil(int outil_id) {
    char msg[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    
    while(1) {
        snprintf(msg, BUFFER_SIZE, "DEMANDE %d", outil_id);
        send(sock_fd, msg, strlen(msg), 0);
        
        memset(response, 0, BUFFER_SIZE);
        recv(sock_fd, response, BUFFER_SIZE - 1, 0);
        
        if(strcmp(response, "OK") == 0) {
            char log[100];
            snprintf(log, sizeof(log), "Outil %d obtenu!", outil_id);
            log_message(log);
            break;
        } else {
            char log[100];
            snprintf(log, sizeof(log), "Attente outil %d...", outil_id);
            log_message(log);
            Sleep(1000);
        }
    }
}

void liberer_outil(int outil_id) {
    char msg[BUFFER_SIZE];
    snprintf(msg, BUFFER_SIZE, "LIBERATION %d", outil_id);
    send(sock_fd, msg, strlen(msg), 0);
    
    char log[100];
    snprintf(log, sizeof(log), "Libere outil %d", outil_id);
    log_message(log);
}

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    
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
    Sleep(id_bras * 1000);
    
    while(1) {
        log_message("=== DEBUT CYCLE ===");
        
        obtenir_outil(0);
        obtenir_outil(1);
        
        log_message(">>> ASSEMBLAGE <<<");
        Sleep(3000);
        
        liberer_outil(1);
        liberer_outil(0);
        
        log_message("=== FIN CYCLE ===\n");
        Sleep(1000);
    }
    
    closesocket(sock_fd);
    WSACleanup();
    return 0;
}