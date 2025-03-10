#include "../headers/orderbook.hpp"
#include "../headers/order.hpp"
#include "../headers/enum.hpp"
#include "../headers/globals.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

OrderBook::OrderBook() : nextOrderId(1) {}

int OrderBook::addOrder(OrderType type, OrderSide side, double price, int quantity, const std::string &symbol) {
    // For Sell orders, check if the account has enough asset.
    if (side == OrderSide::Sell && quantity > account.asset) {
        notifications.push_back("Order Rejected: Insufficient " + symbol + " assets to sell.");
        return -1;
    }
    
    Order newOrder(nextOrderId++, type, side, price, quantity, symbol);

    if (type == OrderType::Market || type == OrderType::Limit) {
        // Attempt to match with orders on the opposite side that share the same asset.
        for (auto &order : orders) {
            if (!order.active) continue;
            if (order.symbol == symbol && order.side != side) {
                // For Market orders, always match.
                // For Limit orders: Buy matches if existing Sell price <= new order price;
                // Sell matches if existing Buy price >= new order price.
                if (type == OrderType::Market ||
                   (side == OrderSide::Buy && order.price <= price) ||
                   (side == OrderSide::Sell && order.price >= price)) {
                    int tradeQty = std::min(newOrder.quantity, order.quantity);
                    std::string notif = "Trade Executed (" + symbol + "): " +
                        (side == OrderSide::Buy ? "Buy " : "Sell ") +
                        std::to_string(tradeQty) + " @ " + std::to_string(order.price);
                    notifications.push_back(notif);
                    // Update account balance.
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
        // If the order isn't fully filled, add the remaining order to the book.
        if (newOrder.quantity > 0) {
            orders.push_back(newOrder);
            return newOrder.id;
        } else {
            return newOrder.id; // Fully executed order.
        }
    } else {
        // For StopLoss and TakeProfit, just add the order.
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
    // Process StopLoss and TakeProfit orders.
    for (auto &order : orders) {
        if (!order.active) continue;
        if (order.type == OrderType::StopLoss) {
            // For Sell StopLoss: trigger if currentPrice <= order.price.
            // For Buy StopLoss: trigger if currentPrice >= order.price.
            if ((order.side == OrderSide::Sell && currentPrice <= order.price) ||
                (order.side == OrderSide::Buy && currentPrice >= order.price)) {
                std::string notif = "StopLoss Triggered (" + order.symbol + "): " + order.toString() +
                    " executed at " + std::to_string(currentPrice);
                notifications.push_back(notif);
                order.active = false;
                // Execute trade at currentPrice.
                if (order.side == OrderSide::Sell) {
                    account.cash += order.quantity * currentPrice;
                    account.asset -= order.quantity;
                } else {
                    account.cash -= order.quantity * currentPrice;
                    account.asset += order.quantity;
                }
            }
        } else if (order.type == OrderType::TakeProfit) {
            // For Sell TakeProfit: trigger if currentPrice >= order.price.
            // For Buy TakeProfit: trigger if currentPrice <= order.price.
            if ((order.side == OrderSide::Sell && currentPrice >= order.price) ||
                (order.side == OrderSide::Buy && currentPrice <= order.price)) {
                std::string notif = "TakeProfit Triggered (" + order.symbol + "): " + order.toString() +
                    " executed at " + std::to_string(currentPrice);
                notifications.push_back(notif);
                order.active = false;
                if (order.side == OrderSide::Sell) {
                    account.cash += order.quantity * currentPrice;
                    account.asset -= order.quantity;
                } else {
                    account.cash -= order.quantity * currentPrice;
                    account.asset += order.quantity;
                }
            }
        }
    }
}
