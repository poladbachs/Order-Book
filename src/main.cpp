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

OrderBook orderBook;

int main()
{
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "OrderBook Engine", NULL, NULL);
    if (window == NULL) return -1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    // Initialize simulation parameters
    double currentPrice = 100.0;
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Input buffers for order entry
    char priceBuffer[32] = "100.0";
    char quantityBuffer[32] = "1";
    char cancelBuffer[32] = "";
    int selectedOrderType = 0;  // 0: Market, 1: Limit, 2: StopLoss, 3: TakeProfit
    int selectedOrderSide = 0;  // 0: Buy, 1: Sell

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        currentPrice += (std::rand() % 100 - 50) / 100.0; // delta between -0.5 and 0.5
        if (currentPrice < 1.0) currentPrice = 1.0; // avoid unrealistic prices

        orderBook.simulateMarket(currentPrice);

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Main window UI
        ImGui::Begin("Order Book Engine");

        // Display simulated market price
        ImGui::Text("Current Market Price: %.2f", currentPrice);
        ImGui::Separator();

        // Section: Add Order
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
            int orderId = orderBook.addOrder(type, side, orderPrice, orderQuantity);
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

        // Section: Active Orders Display
        ImGui::Text("Active Orders:");
        for (const auto &order : orderBook.getOrders())
        {
            if (order.active)
            {
                ImGui::Text("%s", order.toString().c_str());
            }
        }

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
