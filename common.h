#pragma once

#include <stdlib.h>
#include <string>
#include <set>
#include <list>

#include <arpa/inet.h>

void logexit(const char *msg);

int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage);

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);

int server_sockaddr_init(const char *proto, const char *portstr,
                         struct sockaddr_storage *storage);

std::set<std::string> usedTags(const std::string msg);
void strtolist(std::string& msg, std::list<std::string>& out);
