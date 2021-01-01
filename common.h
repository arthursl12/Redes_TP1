#pragma once

#include <stdlib.h>
#include <string>
#include <set>
#include <list>
#include <vector>
#include <map>

#include <arpa/inet.h>

typedef std::map<std::string, std::vector<std::string>> Mapa;
typedef std::pair<std::string, std::vector<std::string>> Par;


void logexit(const char *msg);

int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage);

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);

int server_sockaddr_init(const char *proto, const char *portstr,
                         struct sockaddr_storage *storage);

void strtolist(std::string& msg, std::list<std::string>& out);
void usedtags(std::string& msg, std::set<std::string>& out);

void insert(Mapa& mp, std::string ip_user, std::string tag);
void removeNewLine(char* str);
bool validString(std::string str);