#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <vector>
#include <string>
#include <unordered_map>


struct Account {
    double cash;
    std::unordered_map<std::string, int> assets;
};

extern Account account;
extern std::vector<std::string> notifications;

#endif // GLOBALS_HPP