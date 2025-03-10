#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <vector>
#include <string>

// Account simulation: represents your trading position.
// For example, if trading Bitcoin, 'asset' means your BTC holdings.
struct Account {
    double cash;
    int asset;
};

// Global variables accessible across the project.
extern Account account;
extern std::vector<std::string> notifications;

#endif // GLOBALS_HPP