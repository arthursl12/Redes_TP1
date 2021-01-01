#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "common.h"
#include <set>
#include <string>

TEST_CASE("strtolist"){
    std::string msg = "bloodcyka noob hero #dota";
    std::list<std::string> l1;
    l1.push_back("bloodcyka");
    l1.push_back("noob");
    l1.push_back("hero");
    l1.push_back("#dota");
    std::list<std::string> lr;
    strtolist(msg,lr);
    CHECK(lr == l1);

    msg = "      #bloodcyka       #noob    hero     #dota      ";
    l1.clear();
    l1.push_back("#bloodcyka");
    l1.push_back("#noob");
    l1.push_back("hero");
    l1.push_back("#dota");
    lr.clear();
    strtolist(msg, lr);
    CHECK(lr == l1);

    msg = "#bloodcyka       #noob    hero     #dota      ";
    l1.clear();
    l1.push_back("#bloodcyka");
    l1.push_back("#noob");
    l1.push_back("hero");
    l1.push_back("#dota");
    lr.clear();
    strtolist(msg, lr);
    CHECK(lr == l1);
    
    msg = "   #bloodcyka       #noob    hero     #dota";
    l1.clear();
    l1.push_back("#bloodcyka");
    l1.push_back("#noob");
    l1.push_back("hero");
    l1.push_back("#dota");
    lr.clear();
    strtolist(msg, lr);
    CHECK(lr == l1);
}

TEST_CASE("usedTags"){
    std::string msg = "bloodcyka noob hero #dota";
    std::set<std::string> l1;
    l1.insert("#dota");
    std::set<std::string> lr;
    usedtags(msg, lr);
    CHECK(lr == l1);

    msg = "perdeu eh culpa do suporte #dota #overwatch";
    l1.clear();
    l1.insert("#dota");
    l1.insert("#overwatch");
    lr.clear();
    usedtags(msg, lr);
    CHECK(lr == l1);

    msg = "perdeu #dota eh culpa do suporte #dota #overwatch";
    l1.clear();
    l1.insert("#dota");
    l1.insert("#overwatch");
    lr.clear();
    usedtags(msg, lr);
    CHECK(lr == l1);
    CHECK(lr.size() == l1.size());

    msg = "perdeu eh culpa do suporte #dota#overwatch";
    l1.clear();
    lr.clear();
    usedtags(msg, lr);
    CHECK(lr == l1);
    CHECK(lr.empty() == true);
}

TEST_CASE("Mapa: insert"){
    Mapa mp;
    std::string ip1 = "::1";
    std::string ip2 = "127.0.0.0";
    std::string ip3 = "0.0.0.0";
    std::string ip4 = "2001:db8:3c4d:15::1a2f:1a2b";

    insert(mp, ip1, "#dota");
    CHECK(mp.at(ip1).size() == 1);
    CHECK(mp.at(ip1).at(0) == "#dota");

    insert(mp, ip1, "#dota");
    CHECK(mp.at(ip1).size() == 1);
    CHECK(mp.at(ip1).at(0) == "#dota");

    insert(mp, ip1, "#overwatch");
    CHECK(mp.at(ip1).size() == 2);
    CHECK(mp.at(ip1).at(0) == "#dota");
    CHECK(mp.at(ip1).at(1) == "#overwatch");

    insert(mp, ip2, "#lol");
    CHECK(mp.at(ip2).size() == 1);
    CHECK(mp.at(ip2).at(0) == "#lol");

    insert(mp, ip3, "#lol");
    CHECK(mp.at(ip3).size() == 1);
    CHECK(mp.at(ip3).at(0) == "#lol");

    insert(mp, ip4, "#lol1");
    CHECK(mp.at(ip4).size() == 1);
    CHECK(mp.at(ip4).at(0) == "#lol1");
}