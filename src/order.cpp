#include "../headers/order.hpp"
#include <sstream>

namespace {
    std::string orderTypeToString(OrderType type) {
        switch(type) {
            case OrderType::Market:     return "Market";
            case OrderType::Limit:      return "Limit";
            case OrderType::StopLoss:   return "StopLoss";
            case OrderType::TakeProfit: return "TakeProfit";
            default:                    return "Unknown";
        }
    }
}

Order::Order(int id, OrderType type, double price, int quantity)
    : id(id), type(type), price(price), quantity(quantity), active(true) {}

std::string Order::toString() const {
    std::ostringstream oss;
    oss << "ID: " << id 
        << ", Type: " << orderTypeToString(type)
        << ", Price: " << price 
        << ", Qty: " << quantity 
        << ", Active: " << (active ? "Yes" : "No");
    return oss.str();
}