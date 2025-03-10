#include "../imgui/imgui.h"
#include "../imgui/backends/imgui_impl_glfw.h"
#include "../imgui/backends/imgui_impl_opengl3.h"
#include <../imgui/examples/libs/glfw/include/GLFW/glfw3.h>
#include "../headers/orderbook.hpp"
#include <vector>
#include <string>

OrderBook orderBook;

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required for macOS

    GLFWwindow* window = glfwCreateWindow(800, 600, "OrderBook GUI", NULL, NULL);
    glfwMakeContextCurrent(window);

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");
    ImGui::StyleColorsDark();

    char priceBuffer[32] = "0.0";
    char quantityBuffer[32] = "1";
    int selectedOrderType = 0;
    char cancelBuffer[32] = "";

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("OrderBook");

        const char* orderTypes[] = {"Market", "Limit", "StopLoss", "TakeProfit"};
        ImGui::Combo("Order Type", &selectedOrderType, orderTypes, IM_ARRAYSIZE(orderTypes));
        ImGui::InputText("Price", priceBuffer, IM_ARRAYSIZE(priceBuffer));
        ImGui::InputText("Quantity", quantityBuffer, IM_ARRAYSIZE(quantityBuffer));
        if (ImGui::Button("Add Order")) {
            orderBook.addOrder(static_cast<OrderType>(selectedOrderType), atof(priceBuffer), atoi(quantityBuffer));
        }

        ImGui::Spacing();
        ImGui::InputText("Cancel Order ID", cancelBuffer, IM_ARRAYSIZE(cancelBuffer));
        if (ImGui::Button("Cancel Order")) {
            int idToCancel = atoi(cancelBuffer);
            orderBook.cancelOrder(idToCancel);
        }

        ImGui::Spacing();
        ImGui::Text("Active Orders:");
        for (const auto& order : orderBook.getOrders()) {
            if (order.active) {
                ImGui::Text("ID: %d, Type: %s, Price: %.2f, Qty: %d", order.id, orderTypes[static_cast<int>(order.type)], order.price, order.quantity);
            }
        }

        ImGui::End();
        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}