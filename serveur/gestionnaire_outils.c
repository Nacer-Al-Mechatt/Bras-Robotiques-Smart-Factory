#define _WIN32_WINNT 0x0600
#include "../include/commun.h"

Outil outils[MAX_OUTILS];
BrasRobotique* bras_connectes[MAX_CLIENTS];
int nb_bras = 0;
pthread_mutex_t mutex_global = PTHREAD_MUTEX_INITIALIZER;

void init_outils() {
    for(int i = 0; i < MAX_OUTILS; i++) {
        outils[i].id = i;
        outils[i].etat = OUTIL_LIBRE;
        outils[i].possesseur_id = -1;
        pthread_mutex_init(&outils[i].mutex, NULL);
    }
    printf("[Serveur] %d outils initialises\n", MAX_OUTILS);
}

int traiter_demande(BrasRobotique* bras, int outil_id) {
    pthread_mutex_lock(&outils[outil_id].mutex);
    
    if(outils[outil_id].etat == OUTIL_LIBRE) {
        outils[outil_id].etat = OUTIL_OCCUPE;
        outils[outil_id].possesseur_id = bras->id;
        printf("[Serveur] Bras %d -> prend outil %d\n", bras->id, outil_id);
        pthread_mutex_unlock(&outils[outil_id].mutex);
        return 1;
    }
    
    printf("[Serveur] Bras %d -> outil %d occupe par Bras %d\n", 
           bras->id, outil_id, outils[outil_id].possesseur_id);
    pthread_mutex_unlock(&outils[outil_id].mutex);
    return 0;
}

void traiter_liberation(BrasRobotique* bras, int outil_id) {
    pthread_mutex_lock(&outils[outil_id].mutex);
    
    if(outils[outil_id].possesseur_id == bras->id) {
        outils[outil_id].etat = OUTIL_LIBRE;
        outils[outil_id].possesseur_id = -1;
        printf("[Serveur] Bras %d -> libere outil %d\n", bras->id, outil_id);
    } else {
        printf("[Serveur] Bras %d -> erreur: outil %d n'est pas a lui!\n", bras->id, outil_id);
    }
    
    pthread_mutex_unlock(&outils[outil_id].mutex);
}

void* gerer_client(void* arg) {
    BrasRobotique* bras = (BrasRobotique*)arg;
    char buffer[BUFFER_SIZE];
    int bytes_read;
    
    printf("[Serveur] Bras %d connecte\n", bras->id);
    
    while(1) {
        bytes_read = recv(bras->socket_fd, buffer, BUFFER_SIZE - 1, 0);
        if(bytes_read <= 0) break;
        
        buffer[bytes_read] = '\0';
        
        char cmd[20];
        int id_outil;
        sscanf(buffer, "%s %d", cmd, &id_outil);
        
        if(strcmp(cmd, "DEMANDE") == 0) {
            if(traiter_demande(bras, id_outil)) {
                send(bras->socket_fd, "OK", 2, 0);
                if(bras->nb_outils < 2) {
                    bras->outils_obtenus[bras->nb_outils++] = id_outil;
                }
            } else {
                send(bras->socket_fd, "WAIT", 4, 0);
            }
        }
        else if(strcmp(cmd, "LIBERATION") == 0) {
            traiter_liberation(bras, id_outil);
            send(bras->socket_fd, "OK", 2, 0);
            // Retirer l'outil de la liste du bras
            for(int i = 0; i < bras->nb_outils; i++) {
                if(bras->outils_obtenus[i] == id_outil) {
                    bras->outils_obtenus[i] = -1;
                    break;
                }
            }
        }
    }
    
    for(int i = 0; i < bras->nb_outils; i++) {
        if(bras->outils_obtenus[i] >= 0) {
            traiter_liberation(bras, bras->outils_obtenus[i]);
        }
    }
    
    closesocket(bras->socket_fd);
    printf("[Serveur] Bras %d deconnecte\n", bras->id);
    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    WSADATA wsaData;
    
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    init_outils();
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, MAX_CLIENTS);
    
    printf("[Serveur] Ecoute sur port %d...\n", PORT);
    printf("[Serveur] Attente connexions...\n");
    
    while(1) {
        client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        
        BrasRobotique* bras = (BrasRobotique*)malloc(sizeof(BrasRobotique));
        bras->id = nb_bras;
        bras->socket_fd = client_fd;
        bras->nb_outils = 0;
        memset(bras->outils_obtenus, -1, sizeof(bras->outils_obtenus));
        
        bras_connectes[nb_bras++] = bras;
        
        pthread_t thread_client;
        pthread_create(&thread_client, NULL, gerer_client, bras);
        pthread_detach(thread_client);
        
        printf("[Serveur] Bras %d connecte (total: %d)\n", bras->id, nb_bras);
    }
    
    closesocket(server_fd);
    WSACleanup();
    return 0;
}