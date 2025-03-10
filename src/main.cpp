#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
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

// Define global variables (if not defined elsewhere).
// They are already declared in globals.hpp.
Account account;
std::vector<std::string> notifications;

// Helper to format account as a string with a custom asset label.
std::string accountToString(const Account &acc, const std::string &assetLabel) {
    std::ostringstream oss;
    oss << "Cash: $" << acc.cash << " | " << assetLabel << ": " << acc.asset;
    return oss.str();
}

int main()
{
    // Initialize GLFW with macOS-compatible hints (OpenGL 3.2 Core Profile)
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Order Book Engine", NULL, NULL);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Setup ImGui context and style (modern dark theme)
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    // Use GLSL version 150 (compatible with our macOS setup)
    ImGui_ImplOpenGL3_Init("#version 150");

    // Initialize global account: starting with $10,000 and 0 assets.
    account.cash = 10000.0;
    account.asset = 0;

    // Create a mapping for each asset's current market price.
    // Use realistic starting prices.
    std::unordered_map<std::string, double> assetPrices = {
        {"BTC", 20000.0},
        {"ETH", 1500.0},
        {"AAPL", 150.0},
        {"GOLD", 1800.0}
    };

    // Asset selection: available asset symbols.
    const char* assetList[] = {"BTC", "ETH", "AAPL", "GOLD"};
    int selectedAssetIndex = 0;
    std::string currentAsset = assetList[selectedAssetIndex];

    // Input buffers for order entry
    char priceBuffer[32] = "100.0";
    char quantityBuffer[32] = "1";
    char cancelBuffer[32] = "";
    int selectedOrderType = 0;  // 0: Market, 1: Limit, 2: StopLoss, 3: TakeProfit
    int selectedOrderSide = 0;  // 0: Buy, 1: Sell

    OrderBook orderBook;

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Update current asset based on selection.
        currentAsset = assetList[selectedAssetIndex];
        // Get the current price for the selected asset.
        double currentPrice = assetPrices[currentAsset];

        // Simulate a percentage-based random walk:
        // Delta between -0.5% and +0.5%
        double delta = ((std::rand() % 101) - 50) / 10000.0;
        currentPrice *= (1 + delta);
        // Ensure a minimum price (e.g., 1.0) so it never goes negative.
        if (currentPrice < 1.0) currentPrice = 1.0;
        // Update the map.
        assetPrices[currentAsset] = currentPrice;

        // Process StopLoss and TakeProfit orders based on the current price.
        orderBook.simulateMarket(currentPrice);

        // Start ImGui frame.
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Main UI Window.
        ImGui::Begin("Order Book Engine");

        // Asset selection.
        ImGui::Text("Select Asset:");
        ImGui::Combo("Asset", &selectedAssetIndex, assetList, IM_ARRAYSIZE(assetList));

        // Display current market price and account balance.
        ImGui::Text("Current %s Price: %.2f", currentAsset.c_str(), currentPrice);
        ImGui::Text("%s", accountToString(account, "Asset (" + currentAsset + ")").c_str());
        ImGui::Separator();

        // Section: Place New Order.
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
            // For Sell orders, ensure that you have enough assets.
            if (selectedOrderSide == 1 && orderQuantity > account.asset) {
                notifications.push_back("Order Rejected: Insufficient " + currentAsset + " assets to sell.");
            } else {
                OrderType type = static_cast<OrderType>(selectedOrderType);
                OrderSide side = static_cast<OrderSide>(selectedOrderSide);
                orderBook.addOrder(type, side, orderPrice, orderQuantity, currentAsset);
            }
        }
        ImGui::Separator();

        // Section: Cancel Order.
        ImGui::Text("Cancel Order");
        ImGui::InputText("Order ID", cancelBuffer, IM_ARRAYSIZE(cancelBuffer));
        if (ImGui::Button("Cancel Order"))
        {
            int idToCancel = atoi(cancelBuffer);
            orderBook.cancelOrder(idToCancel);
        }
        ImGui::Separator();

        // Section: Active Orders.
        ImGui::Text("Active Orders:");
        for (const auto &order : orderBook.getOrders())
        {
            if (order.active && order.symbol == currentAsset)
                ImGui::TextWrapped("%s", order.toString().c_str());
        }
        ImGui::Separator();

        // Section: Notifications (scrollable).
        ImGui::Text("Notifications:");
        ImGui::BeginChild("Notifications", ImVec2(0, 150), true);
        for (const auto &notif : notifications)
        {
            ImGui::TextWrapped("%s", notif.c_str());
        }
        ImGui::EndChild();

        ImGui::End();

        // Rendering.
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup.
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
