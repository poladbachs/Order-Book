#ifndef ENUM_HPP
#define ENUM_HPP

// Order types
enum class OrderType {
    Market,
    Limit,
    StopLoss,
    TakeProfit
};

// Order sides
enum class OrderSide {
    Buy,
    Sell
};

#endif // ENUM_HPP