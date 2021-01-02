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

struct cl_thd_args{
    struct client_data* cdata;
    std::vector<int>* cSockS;
};

/*
Remove o soquete do cliente fornecido do vector de soquetes de clientes
*/
void removeClient(std::vector<int>& cSockS, int cl){
    // Procura o soquete do cliente fornecido
    for(auto it = cSockS.begin(); it != cSockS.end(); it++){
        if (*it == cl){
            cSockS.erase(it);
            break;
        }
    }
}

void* client_thread(void* arguments){
    // Processa os argumentos recebidos
    struct cl_thd_args* args = (struct cl_thd_args* )arguments;
    struct client_data* cdata = (struct client_data*) args->cdata;
    struct sockaddr* caddr = (struct sockaddr*)(&cdata->storage);
    std::vector<int>* cSockS = (std::vector<int>*) args->cSockS;

    // Imprime que a conexão teve sucesso
    char caddrstr[BUFSZ];
    addrtostr(caddr, caddrstr, BUFSZ);
    printf("[log] Connection from %s\n", caddrstr);

    bool connected = true;
    while(connected){
        // Receber a mensagem do cliente
        char rec[BUFSZ];
        memset(rec, 0, BUFSZ);
        std::string recStr = "";

        // Receber até encontrar o 'newline' ('\n')
        int count = 0;
        bool newLineFound = false;
        while(!newLineFound){
            count = recv(cdata->csock, rec, BUFSZ-1, 0);

            if (count == 0){
                // Sem mensagens a receber: cliente desligou
                printf("[log] %s closed connection\n", caddrstr);
                connected = false;
                removeClient(*cSockS, cdata->csock);
                break;
            }else if (count < 0){
                // Erro no recv
                logexit("recv (server)");
            }
            
            // Verifica se todos os caracteres da mensagem são válidos
            if (!validString(rec)){
                printf("[log] Invalid message from %s, closing connection\n", caddrstr);
                removeClient(*cSockS, cdata->csock);
                break;
            }

            // Adiciona o que foi recebido 
            recStr += rec;
            std::cout << "[log] Partial string " << recStr << std::endl;

            // Procura newline no que foi recebido
            int find = findNewLine(rec);
            newLineFound = (find != -1) ? true : false;
            std::cout << "[log] Newline at index: " << find << std::endl;
            std::cout.setf(std::ios::boolalpha);
            std::cout << "[log] Found newline: " << newLineFound << std::endl;
        }
        if(!connected){ break;}
        std::cout << "[log] Message received" << std::endl;
        
        // Pós-processamento: caso mais de uma mensagem tenha chegado num mesmo
        // pacote
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
                    removeClient(*cSockS, cdata->csock);
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
    removeClient(*cSockS, cdata->csock);
    close(cdata->csock);
    pthread_exit(EXIT_SUCCESS);
}

void* ping_handler(void* data){
    std::vector<int>* cSockS = (std::vector<int>*) data;
    while(1){
        sleep(2);
        printf("[log] Ping...\n");
        for(int sock: *cSockS){
            char buf[BUFSZ];
            sprintf(buf, "Ping!\n");
            size_t count = send(sock, buf, strlen(buf)+1, 0);
            if (count != strlen(buf)+1){
                logexit("send");
            }
        }
    }
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
    printf("[log] Creating ping_thread\n");
    pthread_t ping_thread;
    pthread_create(&ping_thread, NULL, ping_handler, &cSockS);
    while(1){
        struct sockaddr_storage cstorage;
        struct sockaddr* caddr = (struct sockaddr*)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1){
            logexit("accept");
        }

        // struct client_data *cdata = (struct client_data*) malloc(sizeof(*cdata));
        // if (!cdata){
        //     logexit("malloc");
        // }

        // Argumentos para a thread do cliente
        struct cl_thd_args *args = (struct cl_thd_args*) malloc(sizeof(*args));
        if (!args){
            logexit("malloc");
        }
        struct client_data *cdata = (struct client_data*) malloc(sizeof(*cdata));
        args->cdata = cdata;
        args->cdata->csock = csock;
        memcpy(&(args->cdata->storage), &cstorage, sizeof(cstorage));
        args->cSockS = &cSockS;

        pthread_t tid;
        cSockS.push_back(csock);
        pthread_create(&tid, NULL, client_thread, args);
        
        
    }
    exit(EXIT_SUCCESS);
}

