#include "../headers/orderbook.hpp"
#include "../headers/order.hpp"
#include "../headers/enum.hpp"
#include "../headers/globals.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

OrderBook::OrderBook() : nextOrderId(1) {}

int OrderBook::addOrder(OrderType type, OrderSide side, double price, int quantity) {
    Order newOrder(nextOrderId++, type, side, price, quantity);

    if (type == OrderType::Market || type == OrderType::Limit) {
        // Attempt to match with orders on the opposite side
        for (auto &order : orders) {
            if (!order.active) continue;
            if (order.side != side) { // Opposite side
                if (type == OrderType::Market ||
                   (side == OrderSide::Buy && order.price <= price) ||
                   (side == OrderSide::Sell && order.price >= price)) {
                    int tradeQty = std::min(newOrder.quantity, order.quantity);
                    std::string notif = "Trade Executed: " +
                        std::string((side == OrderSide::Buy ? "Buy " : "Sell ")) +
                        std::to_string(tradeQty) + " @ " + std::to_string(order.price);
                    notifications.push_back(notif);
                    // Update account balance accordingly
                    if (side == OrderSide::Buy) {
                        account.cash -= tradeQty * order.price;
                        account.asset += tradeQty;
                    } else {
                        account.cash += tradeQty * order.price;
                        account.asset -= tradeQty;
                    }
                    newOrder.quantity -= tradeQty;
                    order.quantity -= tradeQty;
                    if (order.quantity == 0) order.active = false;
                    if (newOrder.quantity == 0) break;
                }
            }
        }
        if (newOrder.quantity > 0) {
            orders.push_back(newOrder);
            return newOrder.id;
        } else {
            return newOrder.id;
        }
    } else {
        // For StopLoss and TakeProfit, just add to the book
        orders.push_back(newOrder);
        return newOrder.id;
    }
}

bool OrderBook::cancelOrder(int orderId) {
    for (auto &order : orders) {
        if (order.id == orderId && order.active) {
            order.active = false;
            notifications.push_back("Order Cancelled: ID " + std::to_string(orderId));
            return true;
        }
    }
    return false;
}

std::optional<Order> OrderBook::getOrder(int orderId) const {
    for (const auto &order : orders) {
        if (order.id == orderId && order.active)
            return order;
    }
    return std::nullopt;
}

const std::vector<Order>& OrderBook::getOrders() const {
    return orders;
}

void OrderBook::simulateMarket(double currentPrice) {
    // Process StopLoss and TakeProfit triggers
    for (auto &order : orders) {
        if (!order.active) continue;
        if (order.type == OrderType::StopLoss) {
            // For Sell StopLoss: trigger if currentPrice <= order.price.
            // For Buy StopLoss: trigger if currentPrice >= order.price.
            if ((order.side == OrderSide::Sell && currentPrice <= order.price) ||
                (order.side == OrderSide::Buy && currentPrice >= order.price)) {
                std::string notif = "StopLoss Triggered: " + order.toString() +
                    " at " + std::to_string(currentPrice);
                notifications.push_back(notif);
                order.active = false;
            }
        } else if (order.type == OrderType::TakeProfit) {
            // For Sell TakeProfit: trigger if currentPrice >= order.price.
            // For Buy TakeProfit: trigger if currentPrice <= order.price.
            if ((order.side == OrderSide::Sell && currentPrice >= order.price) ||
                (order.side == OrderSide::Buy && currentPrice <= order.price)) {
                std::string notif = "TakeProfit Triggered: " + order.toString() +
                    " at " + std::to_string(currentPrice);
                notifications.push_back(notif);
                order.active = false;
            }
        }
    }
}
