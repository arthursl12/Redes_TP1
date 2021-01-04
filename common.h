#pragma once

#include <stdlib.h>
#include <string>
#include <set>
#include <list>
#include <vector>
#include <map>

#include <arpa/inet.h>

typedef std::map<std::string, std::vector<std::string>> MapaTag;
typedef std::pair<std::string, std::vector<std::string>> ParTag;

typedef std::map<std::string, int> MapaIpPorta;
typedef std::pair<std::string, int> ParIpPorta;

#define BUFSZ 1024
#define MSGSZ 500


void logexit(const char *msg);

int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage);

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);

int server_sockaddr_init(const char *proto, const char *portstr,
                         struct sockaddr_storage *storage);

void strtolist(std::string& msg, std::list<std::string>& out);

void usedTags(std::string& msg, std::set<std::string>& out);
void notifySet(std::set<std::string>& out, MapaTag& mp, std::string tag);
bool subscribeToTag(MapaTag& mp, std::string caddrtstr, std::string tag);
bool unsubscribeFromTag(MapaTag& mp, std::string caddrtstr, std::string tag);

void removeNewLine(char* str);
bool validString(std::string str);
int findNewLine(std::string str, int pos = 0);

/* Retorna a união dos conjuntos passados */
template <typename T>
std::set<T> getUnion(const std::set<T>& a, const std::set<T>& b){
  std::set<T> result = a;
  result.insert(b.begin(), b.end());
  return result;
}
