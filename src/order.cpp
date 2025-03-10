#include "../headers/order.hpp"
#include "../headers/enum.hpp"
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
    std::string orderSideToString(OrderSide side) {
        return (side == OrderSide::Buy) ? "Buy" : "Sell";
    }
}

Order::Order(int id, OrderType type, OrderSide side, double price, int quantity)
    : id(id), type(type), side(side), price(price), quantity(quantity), active(true) {}

std::string Order::toString() const {
    std::ostringstream oss;
    oss << "ID: " << id 
        << ", " << orderSideToString(side)
        << ", " << orderTypeToString(type)
        << ", Price: " << price 
        << ", Qty: " << quantity 
        << ", Active: " << (active ? "Yes" : "No");
    return oss.str();
}