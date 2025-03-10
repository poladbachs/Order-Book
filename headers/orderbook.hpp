#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include "order.hpp"
#include <vector>
#include <optional>
#include <string>

class OrderBook {
private:
    std::vector<Order> orders;
    int nextOrderId;
public:
    OrderBook();
    // Adds an order (including its asset symbol) and attempts matching.
    int addOrder(OrderType type, OrderSide side, double price, int quantity, const std::string &symbol);
    bool cancelOrder(int orderId);
    std::optional<Order> getOrder(int orderId) const;
    const std::vector<Order>& getOrders() const;
    // Process the current market price and trigger StopLoss/TakeProfit orders.
    void simulateMarket(double currentPrice);
};

#endif // ORDERBOOK_HPP
