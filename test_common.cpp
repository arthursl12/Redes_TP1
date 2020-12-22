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

// TEST_CASE("usedTags"){
//     std::string msg = "bloodcyka noob hero #dota";
//     std::set<std::string> l1 = {"#dota"};
//     CHECK(usedTags(msg) == l1);

//     msg = "perdeu eh culpa do suporte #dota #overwatch";
//     l1 = {"#dota","#overwatch"};
//     CHECK(usedTags(msg) == l1);

//     msg = "perdeu #dota eh culpa do suporte #dota #overwatch";
//     l1 = {"#dota","#overwatch"};
//     CHECK(usedTags(msg) == l1);

//     msg = "perdeu eh culpa do suporte #dota#overwatch";
//     CHECK(usedTags(msg).empty() == true);
// }