#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include "../headers/orderbook.hpp"
#include "../headers/enum.hpp"
#include "../headers/globals.hpp"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <chrono>

// Global variables
Account account;
std::vector<std::string> notifications;

// Each asset gets its own current market price...
std::unordered_map<std::string, double> assetPrices = {
    {"BTC", 70000.0},
    {"ETH", 1500.0},
    {"AAPL", 220.0},
    {"GOLD", 2800.0}
};

// ...and a price history (for the chart)
std::unordered_map<std::string, std::vector<double>> priceHistory;

// Helper to format the account info
std::string accountToString(const Account &acc, const std::string &asset) {
    int assetQty = acc.assets.count(asset) ? acc.assets.at(asset) : 0;
    std::ostringstream oss;
    oss << "Cash: $" << acc.cash << " | " << asset << ": " << assetQty;
    return oss.str();
}

int main()
{
    // GLFW and OpenGL initialization
    if (!glfwInit())
        return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Order Book Engine", NULL, NULL);
    if (!window)
        return -1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // ImGui and ImPlot initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    // Initialize starting cash and clear asset holdings
    account.cash = 100000.0;
    account.assets.clear();

    // Pre-populate each asset’s price history (starting with 100 identical points)
    for (const auto &pair : assetPrices)
        priceHistory[pair.first] = std::vector<double>(100, pair.second);

    // Asset selection list
    const char* assetList[] = {"BTC", "ETH", "AAPL", "GOLD"};
    int selectedAssetIndex = 0;
    std::string currentAsset = assetList[selectedAssetIndex];

    // Buffers and selection for order entry
    char priceBuffer[32] = "100.0";
    char quantityBuffer[32] = "1";
    char cancelBuffer[32] = "";
    int selectedOrderType = 0;  // 0: Market, 1: Limit, 2: StopLoss, 3: TakeProfit
    int selectedOrderSide = 0;  // 0: Buy, 1: Sell

    OrderBook orderBook;
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Use chrono to update the simulation less frequently than every frame.
    auto lastUpdate = std::chrono::steady_clock::now();
    const int updateIntervalMs = 100; // update every 100 ms

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Only update the price simulation at fixed intervals.
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate).count() >= updateIntervalMs)
        {
            // Update the current asset and its price
            currentAsset = assetList[selectedAssetIndex];
            double currentPrice = assetPrices[currentAsset];

            // Use a smaller random change (e.g. ±0.1%) to slow down the movement
            double delta = ((std::rand() % 101) - 50) / 100000.0;
            currentPrice *= (1 + delta);
            if (currentPrice < 1.0)
                currentPrice = 1.0;
            assetPrices[currentAsset] = currentPrice;

            // Update the price history for charting (keeping the last 100 points)
            priceHistory[currentAsset].push_back(currentPrice);
            if (priceHistory[currentAsset].size() > 100)
                priceHistory[currentAsset].erase(priceHistory[currentAsset].begin());

            // Process any StopLoss/TakeProfit orders for the current asset
            orderBook.simulateMarket(currentPrice);

            lastUpdate = now;
        }

        // Start a new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Main UI Window
        ImGui::Begin("Order Book Engine");

        // Asset selection combo
        ImGui::Text("Select Asset:");
        ImGui::Combo("Asset", &selectedAssetIndex, assetList, IM_ARRAYSIZE(assetList));
        currentAsset = assetList[selectedAssetIndex];

        // Display the current market price and account info
        double currentPrice = assetPrices[currentAsset];
        ImGui::Text("Current %s Price: %.2f", currentAsset.c_str(), currentPrice);
        ImGui::Text("%s", accountToString(account, currentAsset).c_str());
        ImGui::Separator();

        // Price chart (using ImPlot)
        if (ImPlot::BeginPlot("Price Chart", ImVec2(-1, 200))) {
            ImPlot::SetupAxes("Time", "Price", 0, ImPlotAxisFlags_AutoFit);
            std::vector<double> xData(priceHistory[currentAsset].size());
            for (size_t i = 0; i < xData.size(); i++)
                xData[i] = i; // Use the index as the X-axis (time) value
            ImPlot::PlotLine(currentAsset.c_str(), xData.data(), priceHistory[currentAsset].data(), xData.size());
            ImPlot::EndPlot();
        }
        ImGui::Separator();

        // Active orders section: show orders that are still active for the current asset
        ImGui::Text("Active Orders:");
        for (const auto &order : orderBook.getOrders()) {
            if (order.active && order.symbol == currentAsset)
                ImGui::TextWrapped("%s", order.toString().c_str());
        }
        ImGui::Separator();

        // Order placement section
        ImGui::Text("Place New Order");
        const char* orderTypes[] = {"Market", "Limit", "StopLoss", "TakeProfit"};
        ImGui::Combo("Order Type", &selectedOrderType, orderTypes, IM_ARRAYSIZE(orderTypes));
        const char* orderSides[] = {"Buy", "Sell"};
        ImGui::Combo("Order Side", &selectedOrderSide, orderSides, IM_ARRAYSIZE(orderSides));
        ImGui::InputText("Price", priceBuffer, IM_ARRAYSIZE(priceBuffer));
        ImGui::InputText("Quantity", quantityBuffer, IM_ARRAYSIZE(quantityBuffer));
        if (ImGui::Button("Place Order"))
        {
            double orderPrice = atof(priceBuffer);
            int orderQuantity = atoi(quantityBuffer);
            // For Sell orders, check if there are enough assets available.
            if (selectedOrderSide == 1 &&
                orderQuantity > (account.assets.count(currentAsset) ? account.assets.at(currentAsset) : 0)) {
                notifications.push_back("Order Rejected: Insufficient " + currentAsset + " assets to sell.");
            } else {
                OrderType type = static_cast<OrderType>(selectedOrderType);
                OrderSide side = static_cast<OrderSide>(selectedOrderSide);
                orderBook.addOrder(type, side, orderPrice, orderQuantity, currentAsset);
            }
        }
        ImGui::Separator();

        // Order cancellation section
        ImGui::Text("Cancel Order");
        ImGui::InputText("Order ID", cancelBuffer, IM_ARRAYSIZE(cancelBuffer));
        if (ImGui::Button("Cancel Order"))
        {
            int idToCancel = atoi(cancelBuffer);
            orderBook.cancelOrder(idToCancel);
        }
        ImGui::Separator();

        // Notifications (scrollable)
        ImGui::Text("Notifications:");
        ImGui::BeginChild("Notifications", ImVec2(0, 150), true);
        for (const auto &notif : notifications)
            ImGui::TextWrapped("%s", notif.c_str());
        ImGui::EndChild();

        ImGui::End();

        // Render frame
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImPlot::DestroyContext();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}