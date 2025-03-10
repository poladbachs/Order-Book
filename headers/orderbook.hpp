#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include "order.hpp"
#include <vector>
#include <optional>

class OrderBook {
private:
    std::vector<Order> orders;
    int nextOrderId;
public:
    OrderBook();
    int addOrder(OrderType type, OrderSide side, double price, int quantity);
    bool cancelOrder(int orderId);
    std::optional<Order> getOrder(int orderId) const;
    const std::vector<Order>& getOrders() const;
    void simulateMarket(double currentPrice);
};

#endif // ORDERBOOK_HPP