#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <list>
#include <set>
#include <string>
#include <iostream>

#include <arpa/inet.h>

void logexit(const char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

int addrparse(const char *addrstr,
              const char *portstr,
              struct sockaddr_storage *storage)
{
    if (addrstr == NULL || portstr == NULL){
        return -1;
    }

    uint16_t port = (uint16_t)atoi(portstr);    // unsigned short
    if (port == 0){
        return -1;
    }
    port = htons(port);     // host to network short

    struct in_addr inaddr4;    // 32-bit IPv4 address
    if (inet_pton(AF_INET, addrstr, &inaddr4)) {
        struct sockaddr_in* addr4 = (struct sockaddr_in*) storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }

    struct in6_addr inaddr6;    // 128-bit IPv6 address
    if (inet_pton(AF_INET6, addrstr, &inaddr6)) {
        struct sockaddr_in6* addr6 = (struct sockaddr_in6*) storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        // addr6->sin6_addr = inaddr6;
        memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
        return 0;
    }

    return -1;
}

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize) {
    int version;
    char addrstr[INET6_ADDRSTRLEN + 1] = "";
    uint16_t port;

    if (addr->sa_family == AF_INET) {
        version = 4;
        struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
        if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr,
                       INET6_ADDRSTRLEN + 1)) {
            logexit("ntop");
        }
        port = ntohs(addr4->sin_port); // network to host short
    } else if (addr->sa_family == AF_INET6) {
        version = 6;
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
        if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr,
                       INET6_ADDRSTRLEN + 1)) {
            logexit("ntop");
        }
        port = ntohs(addr6->sin6_port); // network to host short
    } else {
        logexit("unknown protocol family.");
    }
    if (str) {
        snprintf(str, strsize, "IPv%d %s %hu", version, addrstr, port);
    }
}

int server_sockaddr_init(const char *proto, const char *portstr,
                         struct sockaddr_storage *storage) {
    uint16_t port = (uint16_t)atoi(portstr); // unsigned short
    if (port == 0) {
        return -1;
    }
    port = htons(port); // host to network short

    memset(storage, 0, sizeof(*storage));
    if (0 == strcmp(proto, "v4")) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_addr.s_addr = INADDR_ANY;
        addr4->sin_port = port;
        return 0;
    } else if (0 == strcmp(proto, "v6")) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_addr = in6addr_any;
        addr6->sin6_port = port;
        return 0;
    } else {
        return -1;
    }
}


/*
Transforma uma string em uma lista de palavras separadas por espaços (espaços
múltiplos são considerados como um só)
Adaptado de: 
https://stackoverflow.com/questions/5888022/split-string-by-single-spaces
*/
void strtolist(std::string& msg, std::list<std::string>& out){
    size_t pos = msg.find(' ');
    size_t initialPos = 0;
    out.clear();

    // Quebra string em espaços
    while(pos != std::string::npos) {
        std::string subs = msg.substr(initialPos, pos - initialPos);
        if (subs.size() > 1){
            // Só adiciona à lista se for uma string e não um espaço
            out.push_back(subs);
        }
            
        initialPos = pos + 1;
        pos = msg.find(' ',initialPos);
    }

    // Adicionar a última parte, se não for espaço
    std::string subs = \
        msg.substr(initialPos, std::min(pos, msg.size()) - initialPos + 1);
    if (subs.size() > 1){
        out.push_back(subs);
    }
}

void usedtags(std::string& msg, std::set<std::string>& out){
    std::list<std::string> tags;
    strtolist(msg, tags);
    out.clear();

    for (auto it = tags.begin(); it != tags.end(); it++){
        std::string str = *it; 
        if (str[0] != '#'){
            // Não é uma tag
            continue;
        }else if (str == "##kill"){
            printf("Kill Command");
        }else if (str.find('#',1) != std::string::npos){
            // Uma tag inválida (dois símbolos #)
            continue;
        }else{
            // Insere a tag no conjunto
            out.insert(*it);
        }
    }
}
