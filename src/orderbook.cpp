#include "../headers/orderbook.hpp"
#include "../headers/order.hpp"
#include "../headers/enum.hpp"
#include <algorithm>
#include <iostream>

OrderBook::OrderBook() : nextOrderId(1) {}

// Basic matching engine:
// - For Market orders, match with the best available orders on the opposite side.
// - For Limit orders, if the price crosses the spread, execute matching.
int OrderBook::addOrder(OrderType type, OrderSide side, double price, int quantity) {
    Order newOrder(nextOrderId++, type, side, price, quantity);

    if (type == OrderType::Market || type == OrderType::Limit) {
        // Attempt to match with orders on the opposite side
        for (auto &order : orders) {
            if (!order.active) continue;
            if (order.side != side) { // opposite side
                // For Market orders, always match.
                // For Limit orders: Buy order matches if existing Sell order price <= new order price;
                // Sell order matches if existing Buy order price >= new order price.
                if (type == OrderType::Market ||
                    (side == OrderSide::Buy && order.price <= price) ||
                    (side == OrderSide::Sell && order.price >= price)) {
                    int tradeQty = std::min(newOrder.quantity, order.quantity);
                    std::cout << "Trade Executed: " 
                              << (side == OrderSide::Buy ? "Buy" : "Sell")
                              << " " << tradeQty << " @ " << order.price << std::endl;
                    newOrder.quantity -= tradeQty;
                    order.quantity -= tradeQty;
                    if (order.quantity == 0) order.active = false;
                    if (newOrder.quantity == 0) break;
                }
            }
        }
        // If not fully filled, add remaining order to book
        if (newOrder.quantity > 0) {
            orders.push_back(newOrder);
            return newOrder.id;
        } else {
            return newOrder.id; // Fully executed order (for simulation, still return an ID)
        }
    } else {
        // For StopLoss and TakeProfit orders, just add to the book.
        orders.push_back(newOrder);
        return newOrder.id;
    }
}

bool OrderBook::cancelOrder(int orderId) {
    for (auto &order : orders) {
        if (order.id == orderId && order.active) {
            order.active = false;
            std::cout << "Order Cancelled: ID " << orderId << std::endl;
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

// Simulate market update: trigger StopLoss/TakeProfit orders if conditions met.
void OrderBook::simulateMarket(double currentPrice) {
    for (auto &order : orders) {
        if (!order.active) continue;
        if (order.type == OrderType::StopLoss) {
            // For Sell StopLoss: trigger if currentPrice <= order.price.
            // For Buy StopLoss (rare): trigger if currentPrice >= order.price.
            if ((order.side == OrderSide::Sell && currentPrice <= order.price) ||
                (order.side == OrderSide::Buy && currentPrice >= order.price)) {
                std::cout << "StopLoss Triggered: " << order.toString() 
                          << " at market price " << currentPrice << std::endl;
                order.active = false;
            }
        } else if (order.type == OrderType::TakeProfit) {
            // For Sell TakeProfit: trigger if currentPrice >= order.price.
            // For Buy TakeProfit: trigger if currentPrice <= order.price.
            if ((order.side == OrderSide::Sell && currentPrice >= order.price) ||
                (order.side == OrderSide::Buy && currentPrice <= order.price)) {
                std::cout << "TakeProfit Triggered: " << order.toString() 
                          << " at market price " << currentPrice << std::endl;
                order.active = false;
            }
        }
    }
}
