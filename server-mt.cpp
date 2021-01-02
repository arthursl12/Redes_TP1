#include "common.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

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

    bool connected = true;
    while(connected){
        // Receber a mensagem do cliente
        char rec[BUFSZ];
        memset(rec, 0, BUFSZ);

        std::string recStr = "";

        int count = 0;
        bool newLineFound = false;
        while(!newLineFound){
            count = recv(cdata->csock, rec, BUFSZ-1, 0);

            if (count == 0){
                // Sem mensagens a receber: cliente desligou
                printf("[log] %s closed connection\n", caddrstr);
                connected = false;
                break;
            }else if (count < 0){
                // Erro no recv
                logexit("recv (server)");
            }
            
            // Verifica se todos os caracteres da mensagem são válidos
            if (!validString(rec)){
                printf("[log] Invalid message from %s, closing connection\n", caddrstr);
                break;
            }


            recStr += rec;
            std::cout << "[log] Partial string " << recStr << std::endl;

            int find = findNewLine(rec);
            newLineFound = (find != -1) ? true : false;
            std::cout << "[log] Newline at index: " << find << std::endl;
            std::cout.setf(std::ios::boolalpha);
            std::cout << "[log] Found newline: " << newLineFound << std::endl;
        }
        std::cout << "[log] Message received" << std::endl;
        
        if(!connected){ break;}
        int oldfind = -1;
        int find = findNewLine(recStr.c_str());
        if (find != -1){
            do{
                // Pega só a mensagem entre '\n''s
                std::string bufStr = recStr;
                std::string cpyStr = bufStr.substr(oldfind+1,find-oldfind-1);
                
                // Log mensagem recebida
                printf("[msg] %s, %d bytes: \"%s\"\n", \
                        caddrstr, (int)count, cpyStr.c_str());

                // Verifica se a mensagem é o comando de fim da execução do cliente
                if (strcmp(cpyStr.c_str(),"##quit") == 0){
                    // Comando para fim da execução do cliente, podemos encerrar a 
                    // conexão dele no lado do servidor também
                    printf("[log] %s requested to end connection\n", caddrstr);
                    break;
                }

                // Manda uma confirmação para o cliente
                char buf2[BUFSZ];
                sprintf(buf2, "Message sent sucessfully, %.900s\n", caddrstr);
                count = send(cdata->csock, buf2, strlen(buf2)+1, 0);
                if (count != (int) strlen(buf2)+1){
                    logexit("send");
                }
                
                oldfind = find;
                find = findNewLine(recStr,find+1);
            }while(find != -1);
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

