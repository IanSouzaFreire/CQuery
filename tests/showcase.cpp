#include "CQuery.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
  using namespace CQuery;

  // Set output method
  OutputMethod() = OUT_METHOD::CMD;

  // Find an available port
  int port = findAvailablePort(8000, 9000);
  if (port == -1) {
    std::cerr << "No available port found" << '\n';
    return 1;
  }

  // Create server
  ServerClient server(port);

  // Create _$ instance and bind it to the server
  _$ $;
  $(server);

  // Set the HTML and CSS content for the server
  server.changeContext("example.html", "example.css");

  // Start the server
  server.startServer();

  server.openInBrowser();

  std::cout << "Server running on port " << port << "" << '\n';
  std::cout << "Open a web browser and navigate to http://localhost:" << port << '\n';
  std::cout << "Press Enter to pause the server..." << '\n';
  std::cin.get();

  // Pause the server
  server.pauseServer();

  std::cout << "Server paused. Press Enter to resume..." << '\n';
  std::cin.get();

  // Resume the server
  server.resumeServer();

  std::cout << "Server resumed. Press Enter to update content..." << '\n';
  std::cin.get();

  // Stop the server
  server.closeServer();

  return 0;
}