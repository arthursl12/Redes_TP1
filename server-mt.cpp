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
    Mapa* mp;
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


/*
Função auxiliar que recebe mensagem do cliente (cujos dados estão em 'cdata' e
endereço em 'caddrstr') até encontrar um '\\n'. A mensagem é colocada na string 
'recStr'. 

O vector de soquetes de clientes 'cSockS' é passado para desconectar o cliente 
em questão caso haja algum problema. O booleano 'connected' é colocado em 
'false' caso isso aconteça.
*/
void receberMsg(std::string& recStr, 
                struct client_data* cdata, 
                char caddrstr[],
                std::vector<int>* cSockS, 
                bool& connected,
                int& count)
{
    // Buffer do que foi recebido
    char rec[BUFSZ];
    memset(rec, 0, BUFSZ);
    
    // Receber até encontrar um 'newline' ('\n')
    int total = 0;
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
            connected = false;
            removeClient(*cSockS, cdata->csock);
            break;
        }

        // Adiciona o que foi recebido 
        recStr += rec;
        // std::cout << "[log] Partial string " << recStr << std::endl;
        total += count;

        // Procura newline no que foi recebido
        int find = findNewLine(rec);
        newLineFound = (find != -1) ? true : false;
        std::cout << "[log] Newline at index: " << find << std::endl;
        std::cout.setf(std::ios::boolalpha);
        std::cout << "[log] Found newline: " << newLineFound << std::endl;
    }
    count = total;
}



/*
Função auxiliar que processa a mensagem do cliente (cujos dados estão em 'cdata'
e endereço em 'caddrstr'). A mensagem está na string 'recStr'. Se houver mais de
um newline, será tratado como mais de uma mensagem.

O vector de soquetes de clientes 'cSockS' é passado para desconectar o cliente 
em questão caso haja algum problema. O booleano 'connected' é colocado em 
'false' caso isso aconteça.
*/
void processarMsg(std::string& recStr, 
                struct client_data* cdata, 
                char caddrstr[],
                std::vector<int>* cSockS, 
                bool& connected,
                int& count,
                Mapa* mp)
{
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
                connected = false;
                break;
            }

            // Se for mensagens de (un)subscribe, fazer as alterações
            if (cpyStr[0] == '+'){
                std::cout << "[log] " << caddrstr << " requested to subscribe" << std::endl;
                // Subscribe
                std::string tag = cpyStr.substr(1);
                tag = "#" + tag;
                
                bool newSub = subscribeToTag(*mp, caddrstr, tag);
                if (newSub){
                    // Manda confirmação de inscrição
                    char buf3[BUFSZ];
                    memset(buf3, 0, BUFSZ);
                    sprintf(buf3, "subscribed %s\n", cpyStr.c_str());
                    count = send(cdata->csock, buf3, strlen(buf3)+1, 0);
                    if (count != (int) strlen(buf3)+1){ logexit("send");}
                }else{
                    // Manda aviso que já era inscrito
                    char buf4[BUFSZ];
                    memset(buf4, 0, BUFSZ);
                    sprintf(buf4, "already subscribed %s\n", cpyStr.c_str());
                    count = send(cdata->csock, buf4, strlen(buf4)+1, 0);
                    if (count != (int) strlen(buf4)+1){ logexit("send");}
                }
            }else if(cpyStr[0] == '-'){
                // Unsubscribe
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
            // Processa a mensagem seguinte se houver mais de um '\n'
        }while(find != -1);
    }
}

/*
Thread de um cliente. Recebe e processa mensagens recebidas.
*/
void* client_thread(void* arguments){
    // Processa os argumentos recebidos
    struct cl_thd_args* args = (struct cl_thd_args* )arguments;
    struct client_data* cdata = (struct client_data*) args->cdata;
    struct sockaddr* caddr = (struct sockaddr*)(&cdata->storage);
    std::vector<int>* cSockS = (std::vector<int>*) args->cSockS;
    Mapa* mp = args->mp;

    // Imprime que a conexão teve sucesso
    char caddrstr[BUFSZ];
    addrtostr(caddr, caddrstr, BUFSZ);
    printf("[log] Connection from %s\n", caddrstr);

    bool connected = true;
    while(connected){
        // Receber a mensagem do cliente até encontrar o newline
        int count = 0;
        std::string recStr = "";
        receberMsg(recStr, cdata, caddrstr, cSockS, connected, count);

        if(!connected){ break;}
        std::cout << "[log] Message received" << std::endl;
        
        // Processa a mensagem (separa mensagens múltiplas)
        processarMsg(recStr, cdata, caddrstr, cSockS, connected, count, mp);
    }
    removeClient(*cSockS, cdata->csock);
    close(cdata->csock);
    pthread_exit(EXIT_SUCCESS);
}

void* ping_handler(void* data){
    std::vector<int>* cSockS = (std::vector<int>*) data;
    while(1){
        sleep(10);
        printf("[log] Ping...\n");
        for(int sock: *cSockS){
            char buf[BUFSZ];
            memset(buf, 0, BUFSZ);
            sprintf(buf, "\nPing!\n");
            size_t count = send(sock, buf, strlen(buf)+1, 0);
            if (count != strlen(buf)+1){
                logexit("send");
            }
        }
    }
}

int main(int argc, char* argv[]){
    // Processa os argumentos passados
    if (argc < 3){
        usage(argc, argv);
    }
    struct sockaddr_storage storage;
    if (server_sockaddr_init(argv[1], argv[2], &storage) != 0){
        usage(argc, argv);
    }
    
    // Socket do servidor
    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) { logexit("socket");}
    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))){
        logexit("setsockopt");
    }

    // Inicializa o servidor e aguarda conexões dos clientes
    struct sockaddr* addr = (struct sockaddr*)(&storage);
    if (bind(s, addr, sizeof(storage)) != 0){ logexit("bind");}
    if (listen(s, 10) != 0){ logexit("listen");}
    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("[log] Bound to %s, waiting connections...\n", addrstr);

    std::vector<int> cSockS;
    Mapa mp;     // Mapa que guarda as tags dos clientes
    printf("[log] Creating ping_thread\n");
    pthread_t ping_thread;
    pthread_create(&ping_thread, NULL, ping_handler, &cSockS);
    
    // Loop principal
    while(1){
        struct sockaddr_storage cstorage;
        struct sockaddr* caddr = (struct sockaddr*)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        // Aceita a conexão do cliente
        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1){ logexit("accept");}

        // Argumentos para a thread do cliente
        struct cl_thd_args *args = (struct cl_thd_args*) malloc(sizeof(*args));
        if (!args){ logexit("malloc");}
        struct client_data *cdata = (struct client_data*) malloc(sizeof(*cdata));
        args->cdata = cdata;
        args->cdata->csock = csock;
        memcpy(&(args->cdata->storage), &cstorage, sizeof(cstorage));
        args->cSockS = &cSockS;
        args->mp = &mp;

        // Conecta o cliente via uma thread
        pthread_t tid;
        cSockS.push_back(csock);
        pthread_create(&tid, NULL, client_thread, args);
    }
    exit(EXIT_SUCCESS);
}

