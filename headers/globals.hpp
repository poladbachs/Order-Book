#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <vector>
#include <string>

// Simulated account structure
struct Account {
    double cash;
    int asset;
};

extern Account account;
extern std::vector<std::string> notifications;

#endif 