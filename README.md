![Image](https://github.com/user-attachments/assets/4d989fb2-6da7-4737-b9e9-501886684796)

# C++ Order Book Engine

A C++ pet project that simulates a realistic order book engine (Crypto, Commodities, Stocks) for algo trading. The engine supports multiple asset types with independent price simulations, various order types (Market, Limit, StopLoss, TakeProfit), and active order management—all showcased in a modern GUI with real‑time charts.

---

## 📺 Demo
![Image](https://github.com/user-attachments/assets/8f18b1b9-9447-49e8-afe2-45fead63bd54)

---

## 📌 How It Works
- **Market Simulation:**
  Each asset’s price (BTC, ETH, AAPL, GOLD) is updated using a controlled random walk to simulate realistic price movements within predefined boundaries.

- **Order Processing:**
  When a new order is placed, it’s matched against existing orders on the opposite side. Any unfilled quantity remains in the order book until cancelled or fully executed.

- **Account & Notifications:**
Trade executions update a simulated account’s cash and asset holdings. Notifications are generated for trade executions, order triggers, and cancellations.

- **Interactive UI: Dear ImGUI & Implot charts:**

  - Asset Selection: Choose between different assets (BTC, ETH, AAPL, GOLD).
  - Real-Time Charts: View live price history using ImPlot.
  - Order Entry: Place and cancel orders through a clean, modern interface.
  - Notifications Panel: Stay informed of order events and trade executions.

---

## 🛠️ Tech Stack

- **Language:** C++ (C++17)
- **GUI & Plotting:** Dear ImGui, ImPlot
- **Graphics & Windowing:** OpenGL, GLFW

---

## 🔧 Setup & Build

1. **Install Dependencies (macOS):**
   ```bash
   brew install glfw
   ```
2. **Clone Required Libraries:**
   ```bash
   git clone https://github.com/ocornut/imgui.git
   git clone https://github.com/epezent/implot.git
   ```
3. **Compile the Project:**
   ```bash
   g++ -std=c++17 -DGL_SILENCE_DEPRECATION -o OrderBook \
    src/main.cpp src/order.cpp src/orderbook.cpp \
    imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_widgets.cpp imgui/imgui_tables.cpp \
    imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp \
    implot/implot.cpp implot/implot_items.cpp \
    -Iimgui -Iimgui/backends -Iimplot -I/usr/local/include \
    -L/usr/local/lib -lglfw -framework OpenGL
   ```
4. **Run the Application:**
   ```bash
   ./OrderBook
   ```
