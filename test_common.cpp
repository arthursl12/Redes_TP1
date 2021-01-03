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
    usedTags(msg, lr);
    CHECK(lr == l1);

    msg = "perdeu eh culpa do suporte #dota #overwatch";
    l1.clear();
    l1.insert("#dota");
    l1.insert("#overwatch");
    lr.clear();
    usedTags(msg, lr);
    CHECK(lr == l1);

    msg = "perdeu #dota eh culpa do suporte #dota #overwatch";
    l1.clear();
    l1.insert("#dota");
    l1.insert("#overwatch");
    lr.clear();
    usedTags(msg, lr);
    CHECK(lr == l1);
    CHECK(lr.size() == l1.size());

    msg = "perdeu eh culpa do suporte #dota#overwatch";
    l1.clear();
    lr.clear();
    usedTags(msg, lr);
    CHECK(lr == l1);
    CHECK(lr.empty() == true);
}

TEST_CASE("Mapa: subscribeToTag"){
    MapaTag mp;
    std::string ip1 = "::1 2001";

    CHECK(subscribeToTag(mp, ip1, "#dota"));
    CHECK(mp.at(ip1).size() == 1);
    CHECK(mp.at(ip1).at(0) == "#dota");

    CHECK_FALSE(subscribeToTag(mp, ip1, "#dota"));
    CHECK(mp.at(ip1).size() == 1);
    CHECK(mp.at(ip1).at(0) == "#dota");

    CHECK(subscribeToTag(mp, ip1, "#overwatch"));
    CHECK(mp.at(ip1).size() == 2);
    CHECK(mp.at(ip1).at(0) == "#dota");
    CHECK(mp.at(ip1).at(1) == "#overwatch");
}

TEST_CASE("Mapa: subscribeToTag - diferentes tipos IPs"){
    MapaTag mp;
    std::string ip1 = "::1 2001";
    std::string ip2 = "::1 2002";
    std::string ip3 = "0.0.0.0 2001";
    std::string ip4 = "0.0.0.0 2002";
    std::string ip5 = "2001:db8:3c4d:15::1a2f:1a2b 2001";
    std::string ip6 = "2001:db8:3c4d:15::1a2f:1a2b 2002";

    SUBCASE("IPv6 abreviado"){
        subscribeToTag(mp, ip1, "#dota");
        CHECK(mp.at(ip1).size() == 1);
        CHECK(mp.at(ip1).at(0) == "#dota");

        subscribeToTag(mp, ip2, "#lol");
        CHECK(mp.at(ip2).size() == 1);
        CHECK(mp.at(ip2).at(0) == "#lol");

        CHECK(subscribeToTag(mp, ip1, "#overwatch"));
        CHECK(mp.at(ip1).size() == 2);
        CHECK(mp.at(ip1).at(0) == "#dota");
        CHECK(mp.at(ip1).at(1) == "#overwatch");
    }
    SUBCASE("IPv4"){
        CHECK(subscribeToTag(mp, ip3, "#lol"));
        CHECK(mp.at(ip3).size() == 1);
        CHECK(mp.at(ip3).at(0) == "#lol");

        CHECK(subscribeToTag(mp, ip4, "#lol1"));
        CHECK(mp.at(ip4).size() == 1);
        CHECK(mp.at(ip4).at(0) == "#lol1");

        CHECK(subscribeToTag(mp, ip3, "#overwatch"));
        CHECK(mp.at(ip3).size() == 2);
        CHECK(mp.at(ip3).at(0) == "#lol");
        CHECK(mp.at(ip3).at(1) == "#overwatch");
    }
    SUBCASE("IPv6 completo"){
        CHECK(subscribeToTag(mp, ip5, "#lol"));
        CHECK(mp.at(ip5).size() == 1);
        CHECK(mp.at(ip5).at(0) == "#lol");

        CHECK(subscribeToTag(mp, ip6, "#lol1"));
        CHECK(mp.at(ip6).size() == 1);
        CHECK(mp.at(ip6).at(0) == "#lol1");

        CHECK(subscribeToTag(mp, ip5, "#overwatch"));
        CHECK(mp.at(ip5).size() == 2);
        CHECK(mp.at(ip5).at(0) == "#lol");
        CHECK(mp.at(ip5).at(1) == "#overwatch");
    }
}

TEST_CASE("Mapa: unsubscribeFromTag"){
    MapaTag mp;
    std::string ip1 = "::1 2001";

    CHECK(mp.size() == 0);
    CHECK_FALSE(unsubscribeFromTag(mp, ip1, "#overwatch"));
    CHECK_FALSE(unsubscribeFromTag(mp, ip1, "#dota"));

    subscribeToTag(mp, ip1, "#dota");
    CHECK_FALSE(unsubscribeFromTag(mp, ip1, "#overwatch"));
    
    subscribeToTag(mp, ip1, "#overwatch");
    CHECK(mp.at(ip1).size() == 2);
    CHECK(mp.at(ip1).at(0) == "#dota");
    CHECK(mp.at(ip1).at(1) == "#overwatch");

    subscribeToTag(mp, ip1, "#lol");
    CHECK(mp.at(ip1).size() == 3);
    CHECK(mp.at(ip1).at(0) == "#dota");
    CHECK(mp.at(ip1).at(1) == "#overwatch");
    CHECK(mp.at(ip1).at(2) == "#lol");


    CHECK(unsubscribeFromTag(mp, ip1, "#dota"));
    CHECK(mp.at(ip1).size() == 2);
    CHECK(mp.at(ip1).at(0) == "#overwatch");
    CHECK(mp.at(ip1).at(1) == "#lol");
    CHECK_FALSE(unsubscribeFromTag(mp, ip1, "#dota"));

    CHECK(mp.at(ip1).size() == 2);
    CHECK(mp.at(ip1).at(0) == "#overwatch");
    CHECK(mp.at(ip1).at(1) == "#lol");

    CHECK(unsubscribeFromTag(mp, ip1, "#overwatch"));
    CHECK(mp.at(ip1).size() == 1);
    CHECK(mp.at(ip1).at(0) == "#lol");
}

TEST_CASE("validateString"){
    CHECK(validString(" "));
    CHECK(validString("\t"));
    CHECK(validString(" 1234"));
    CHECK(validString("\tt\nABCDF "));
    CHECK(validString("#.90 #dota #anonovo\t\t,\t()"));
    CHECK_FALSE(validString(" \\"));
    CHECK(validString(",.?!:;+-*/=@#$%()[]{}"));
    CHECK_FALSE(validString("_underscore_"));
    CHECK_FALSE(validString("_u&nderscore_"));
    CHECK_FALSE(validString("áéíóúiüöèàããõõôâêî"));
    CHECK_FALSE(validString("çÇ"));
    CHECK_FALSE(validString("¬~<>^´´`|"));
}

TEST_CASE("findNewLine"){
    CHECK(findNewLine("") == -1);
    CHECK(findNewLine("aaa") == -1);
    CHECK(findNewLine("   ") == -1);
    CHECK(findNewLine("\t\tA90") == -1);
    CHECK(findNewLine("\n") == 0);
    CHECK(findNewLine("ABCD\nABSDC") == 4);
    CHECK(findNewLine("ABCD\nABSDC",4) == 4);
    CHECK(findNewLine("ABCD\nABSDC",5) == -1);
    CHECK(findNewLine("ABCD\nABSDC",20) == -1);
}

TEST_CASE("Multiple Messages one package"){
    SUBCASE("Uma: caso comum"){
        int BUFSZ = 1024;
        char buf[BUFSZ] = "boa tarde #MaisUmDia\n";

        int find = findNewLine(buf);
        if (find != -1){
            do{
                // Log mensagem recebida
                char cpy[BUFSZ];
                strcpy(cpy, buf);
                cpy[find] = '\0';

                CHECK(std::string(cpy) == "boa tarde #MaisUmDia");

                // int oldfind = find;
                find = findNewLine(buf,find+1);
                CHECK(find == -1);
            }while(find != -1);
        }
    }
    SUBCASE("Duas"){
        int BUFSZ = 1024;
        char buf[BUFSZ] = "boa tarde #MaisUmDia\nbom almoço #DiarioAlimentar\n";

        int i = 0;

        int oldfind = -1;
        int find = findNewLine(buf);
        if (find != -1){
            do{
                // Log mensagem recebida
                std::string bufStr = buf;
                std::string cpyStr = bufStr.substr(oldfind+1,find-oldfind-1);

                if (i == 0){
                    CHECK(cpyStr == "boa tarde #MaisUmDia");
                    i++;
                }else if (i == 1){
                    CHECK(cpyStr == "bom almoço #DiarioAlimentar");
                    i++;
                }else{
                    CHECK(false);
                }
                
                oldfind = find;
                find = findNewLine(buf,find+1);
            }while(find != -1);
        }
    }
    SUBCASE("Três"){
        int BUFSZ = 1024;
        char buf[BUFSZ] = "boa tarde #MaisUmDia\nbom almoço #DiarioAlimentar\nboa janta #DiarioAlimentar3\n";

        int i = 0;

        int oldfind = -1;
        int find = findNewLine(buf);
        if (find != -1){
            do{
                // Log mensagem recebida
                std::string bufStr = buf;
                std::string cpyStr = bufStr.substr(oldfind+1,find-oldfind-1);

                if (i == 0){
                    CHECK(cpyStr == "boa tarde #MaisUmDia");
                    i++;
                }else if (i == 1){
                    CHECK(cpyStr == "bom almoço #DiarioAlimentar");
                    i++;
                }else if (i == 2){
                    CHECK(cpyStr == "boa janta #DiarioAlimentar3");
                    i++;
                }else{
                    CHECK(false);
                }
                
                oldfind = find;
                find = findNewLine(buf,find+1);
            }while(find != -1);
        }
    }
    
}

TEST_CASE("NotifySet"){
    MapaTag mp;
    std::string ip1 = "::1 2001";
    std::string ip2 = "::1 2002";
    std::string ip3 = "0.0.0.0 2001";

    subscribeToTag(mp, ip1, "#overwatch");
    subscribeToTag(mp, ip1, "#lol");
    subscribeToTag(mp, ip1, "#dota");
    subscribeToTag(mp, ip1, "#TBT");

    subscribeToTag(mp, ip2, "#TBT");

    subscribeToTag(mp, ip3, "#TBT");
    subscribeToTag(mp, ip3, "#overwatch");

    std::set<std::string> out;
    notifySet(out, mp, "#TBT");
    CHECK(out.size() == 3);
    CHECK(out.find(ip1) != out.end());
    CHECK(out.find(ip2) != out.end());
    CHECK(out.find(ip3) != out.end());

    out.clear();
    notifySet(out, mp, "#overwatch");
    CHECK(out.size() == 2);
    CHECK(out.find(ip1) != out.end());
    CHECK(out.find(ip2) == out.end());
    CHECK(out.find(ip3) != out.end());

    out.clear();
    notifySet(out, mp, "#lol");
    CHECK(out.size() == 1);
    CHECK(out.find(ip1) != out.end());
    CHECK(out.find(ip2) == out.end());
    CHECK(out.find(ip3) == out.end());

    out.clear();
    notifySet(out, mp, "#dota");
    CHECK(out.size() == 1);
    CHECK(out.find(ip1) != out.end());
    CHECK(out.find(ip2) == out.end());
    CHECK(out.find(ip3) == out.end());

    out.clear();
    notifySet(out, mp, "#ksp");
    CHECK(out.size() == 0);










}