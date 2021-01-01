#include "common.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#define BUFSZ 1024


void usage(int argc, char* argv[]){
    printf("usage: %s <v4|v6> <server_port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

struct client_data{
    int csock;
    struct sockaddr_storage storage;
};

void* client_thread(void* data){
    struct client_data* cdata = (struct client_data*) data;
    struct sockaddr* caddr = (struct sockaddr*)(&cdata->storage);

    
    char caddrstr[BUFSZ];
    addrtostr(caddr, caddrstr, BUFSZ);
    printf("[log] connection from %s\n", caddrstr);

    while(1){
        // Receber a mensagem do cliente
        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);
        int count = recv(cdata->csock, buf, BUFSZ-1, 0);

        if (count == 0){
            // Sem mensagens a receber: cliente desligou
            printf("[log] %s closed connection\n", caddrstr);
            break;
        }else{
            // Erro no recv
            logexit("recv (server)");
        }
        
        // Verifica se todos os caracteres da mensagem são válidos
        if (!validString(buf)){
            printf("[log] Invalid message from %s, closing connection\n", caddrstr);
            break;
        }
        
        // Log mensagem recebida
        removeNewLine(buf);
        printf("[msg] %s, %d bytes: \"%s\"\n", caddrstr, (int)count, buf);

        // Verifica se a mensagem é o comando de fim da execução do cliente
        if (strcmp(buf,"##quit") == 0){
            // Comando para fim da execução do cliente, podemos encerrar a 
            // conexão dele no lado do servidor também
            printf("[log] %s requested to end connection\n", caddrstr);
            break;
        }

        // Manda uma confirmação para o cliente
        sprintf(buf, "Message sent sucessfully, %.900s\n", caddrstr);
        count = send(cdata->csock, buf, strlen(buf)+1, 0);
        if (count != strlen(buf)+1){
            logexit("send");
        }
    }
    close(cdata->csock);
    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]){
    if (argc < 3){
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (server_sockaddr_init(argv[1], argv[2], &storage) != 0){
        usage(argc, argv);
    }
    
    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))){
        logexit("setsockopt");
    }

    struct sockaddr* addr = (struct sockaddr*)(&storage);
    if (bind(s, addr, sizeof(storage)) != 0){
        logexit("bind");
    }

    if (listen(s, 10) != 0){
        logexit("listen");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting connections\n", addrstr);

    std::vector<int> cSockS;
    while(1){
        struct sockaddr_storage cstorage;
        struct sockaddr* caddr = (struct sockaddr*)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1){
            logexit("accept");
        }

        struct client_data *cdata = (struct client_data*) malloc(sizeof(*cdata));
        if (!cdata){
            logexit("malloc");
        }
        cdata->csock = csock;
        memcpy(&(cdata->storage), &cstorage, sizeof(cstorage));

        pthread_t tid;
        cSockS.push_back(csock);
        printf("[log] size of vector of clients: %lu\n", cSockS.size());
        pthread_create(&tid, NULL, client_thread, cdata);

        for(int sock: cSockS){
            printf("Sock: %i\n",sock);
            // char buf[BUFSZ];
            // sprintf(buf, "Ping!\n");
            // size_t count = send(sock, buf, strlen(buf)+1, 0);
            // if (count != strlen(buf)+1){
            //     logexit("send");
            // }
        }
    }
    exit(EXIT_SUCCESS);
}

