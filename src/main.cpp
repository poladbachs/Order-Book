#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "../headers/orderbook.hpp"
#include "../headers/enum.hpp"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <sstream>

// Global notifications vector (for UI messages)
std::vector<std::string> notifications;

// Account simulation: track cash and asset holdings
struct Account {
    double cash;
    int asset;
};
Account account = {10000.0, 0};  // Starting with $10,000 cash and 0 assets

// Helper to format account balance as string
std::string accountToString(const Account &acc) {
    std::ostringstream oss;
    oss << "Cash: $" << acc.cash << " | Asset: " << acc.asset;
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
    // Use GLSL version 150 which is compatible with our setup on macOS
    ImGui_ImplOpenGL3_Init("#version 150");

    // Initialize simulated market variables
    double currentPrice = 100.0;  // Starting market price
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Input buffers for order entry
    char priceBuffer[32] = "100.0";
    char quantityBuffer[32] = "1";
    char cancelBuffer[32] = "";
    int selectedOrderType = 0;  // 0: Market, 1: Limit, 2: StopLoss, 3: TakeProfit
    int selectedOrderSide = 0;  // 0: Buy, 1: Sell

    OrderBook orderBook;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Simulate market price update (simple random walk)
        currentPrice += (std::rand() % 100 - 50) / 100.0; // Delta between -0.5 and 0.5
        if (currentPrice < 1.0) currentPrice = 1.0;

        // Process stop-loss and take-profit triggers based on current market price
        orderBook.simulateMarket(currentPrice);

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Main UI Window
        ImGui::Begin("Order Book Engine");

        // Display simulated market price and account balance
        ImGui::Text("Current Market Price: %.2f", currentPrice);
        ImGui::Text("%s", accountToString(account).c_str());
        ImGui::Separator();

        // Section: Place New Order
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
            OrderType type = static_cast<OrderType>(selectedOrderType);
            OrderSide side = static_cast<OrderSide>(selectedOrderSide);
            orderBook.addOrder(type, side, orderPrice, orderQuantity);
        }
        ImGui::Separator();

        // Section: Cancel Order
        ImGui::Text("Cancel Order");
        ImGui::InputText("Order ID", cancelBuffer, IM_ARRAYSIZE(cancelBuffer));
        if (ImGui::Button("Cancel Order"))
        {
            int idToCancel = atoi(cancelBuffer);
            orderBook.cancelOrder(idToCancel);
        }
        ImGui::Separator();

        // Section: Active Orders
        ImGui::Text("Active Orders:");
        for (const auto &order : orderBook.getOrders())
        {
            if (order.active)
                ImGui::TextWrapped("%s", order.toString().c_str());
        }
        ImGui::Separator();

        // Section: Notifications (scrollable)
        ImGui::Text("Notifications:");
        ImGui::BeginChild("Notifications", ImVec2(0, 150), true);
        for (const auto &notif : notifications)
        {
            ImGui::TextWrapped("%s", notif.c_str());
        }
        ImGui::EndChild();

        ImGui::End();

        // Rendering
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
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}