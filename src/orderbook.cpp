#include "../headers/orderbook.hpp"
#include <algorithm>

OrderBook::OrderBook() : nextOrderId(1) {}

int OrderBook::addOrder(OrderType type, double price, int quantity) {
    Order newOrder(nextOrderId++, type, price, quantity);
    orders.push_back(newOrder);
    return newOrder.id;
}

bool OrderBook::cancelOrder(int orderId) {
    auto it = std::find_if(orders.begin(), orders.end(), [orderId](Order &o) {
        return o.id == orderId && o.active;
    });
    if (it != orders.end()) {
        it->active = false;
        return true;
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