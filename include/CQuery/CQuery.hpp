#ifndef SERVERCLIENT_HPP
#define SERVERCLIENT_HPP

#ifdef _WIN32
#include <winsock2.h>
#include <Windows.h>
#include <Ws2tcpip.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <unistd.h>
// #pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <exception>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <thread>
#include <atomic>

namespace CQuery {

enum class OUT_METHOD {
  _SET_,
  _OUT_,
  CMD,
  FILE,
  SILENT
};

OUT_METHOD& OutputMethod() {
  static OUT_METHOD current = OUT_METHOD::CMD;
  return current;
}

std::ostream& Output() {
  switch (OutputMethod()) {
    case OUT_METHOD::CMD:
      return std::cout;
    case OUT_METHOD::FILE:
      static std::ofstream fileStream("h.log");
      return fileStream;
    case OUT_METHOD::SILENT:
      static std::ostream nullStream(nullptr);
      return nullStream;
    default:
      return std::cout;
  }
}

#ifdef _WIN32
void initWinsock()
{
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
  {
    throw std::runtime_error("Error initializing Winsock.");
  }
}
#endif

int findAvailablePort(int startPort, int endPort)
{
#ifdef _WIN32
  initWinsock();
#endif

  for (int port = startPort; port <= endPort; port++) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
#ifdef _WIN32
      std::cerr << "Error creating socket: " << WSAGetLastError() << '\n';
#else
      std::cerr << "Error creating socket: " << strerror(errno) << " (errno: " << errno << ")" << '\n';
#endif
      continue;  // Try the next port instead of returning immediately
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
      close(sockfd);
      return port;
    }

    close(sockfd);
  }

  return -1; // no available port found
}

template <class fun_t>
fun_t proc(fun_t (*fun))
{ return fun(); }

template <class fun_t, class F>
fun_t proc(F&& fun)
{ return fun(); }

class ServerClient {
  int serverSocket_;
  struct sockaddr_in serverAddress_;
  struct sockaddr_in clientAddress_;
#ifdef _WIN32
  int clientAddressLength_;
#else
  socklen_t clientAddressLength_;
#endif
  std::string htmlFile_;
  std::string cssFile_;
  std::thread serverThread_;
  std::atomic<bool> running_;
  std::atomic<bool> paused_;
  std::condition_variable pauseCondition_;
  std::mutex pauseMutex_;

  std::string readFile(const std::string& filePath)
  {
    std::ifstream file(filePath);
    if (!file.is_open()) {
      throw std::runtime_error("Unable to open file: " + filePath);
    }
    return std::string((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
  }
public:
  ServerClient(int port) : paused_(false)
  {
#ifdef _WIN32
    initWinsock();
#endif
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket_ < 0)
    {
#ifdef _WIN32
      throw std::runtime_error("Error creating server socket: " + std::to_string(WSAGetLastError()));
#else
      throw std::runtime_error("Error creating server socket: " + std::string(strerror(errno)));
#endif
    }

    serverAddress_.sin_family = AF_INET;
    serverAddress_.sin_port = htons(port);
    serverAddress_.sin_addr.s_addr = INADDR_ANY;  // Changed this line

    // Add this: allow reuse of the address
    int opt = 1;
    if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) < 0)
    {
#ifdef _WIN32
      throw std::runtime_error("Error setting socket option: " + std::to_string(WSAGetLastError()));
#else
      throw std::runtime_error("Error setting socket option: " + std::string(strerror(errno)));
#endif
    }

    if (bind(serverSocket_, (struct sockaddr*)&serverAddress_, sizeof(serverAddress_)) < 0)
    {
#ifdef _WIN32
      throw std::runtime_error("Error binding server socket to address: " + std::to_string(WSAGetLastError()));
#else
      throw std::runtime_error("Error binding server socket to address: " + std::string(strerror(errno)));
#endif
    }

    if (listen(serverSocket_, 3) < 0)
    {
#ifdef _WIN32
      throw std::runtime_error("Error listening on server socket: " + std::to_string(WSAGetLastError()));
#else
      throw std::runtime_error("Error listening on server socket: " + std::string(strerror(errno)));
#endif
    }

    running_ = false;
  }
  
  ~ServerClient()
  {
    closeServer();
#ifdef _WIN32
    closesocket(serverSocket_);
    WSACleanup();
#else
    close(serverSocket_);
#endif
  }

  void startServer()
  {
    running_ = true;
    serverThread_ = std::thread(&ServerClient::serverLoop, this);
    Output() << "Server started. Listening for incoming connections..." << '\n';
  }

  void closeServer()
  {
    running_ = false;
    if (serverThread_.joinable()) {
      serverThread_.join();
    }
    Output() << "Server stopped." << '\n';
  }

  void pauseServer()
  {
    paused_ = true;
    Output() << "Server paused." << '\n';
  }

  void resumeServer()
  {
    paused_ = false;
    pauseCondition_.notify_all();
    Output() << "Server resumed." << '\n';
  }

  void handleClient(int clientSocket)
  {
    Output() << "Client connected." << '\n';

    std::vector<char> buffer(4096);
    int bytesRead = recv(clientSocket, buffer.data(), buffer.size(), 0);
    if (bytesRead < 0)
    {
      Output() << "Error receiving data from client." << '\n';
      return;
    }

    std::string request(buffer.data(), bytesRead);
    std::string response;
    std::string contentType;

    if (request.find("GET /style.css") != std::string::npos) {
      // Serve CSS content
      contentType = "text/css";
      response = cssFile_;
    } else {
      // Serve HTML content by default
      contentType = "text/html";
      response = htmlFile_;
    }

    std::string header = "HTTP/1.1 200 OK\r\n";
    header += "Content-Type: " + contentType + "\r\n";
    header += "Content-Length: " + std::to_string(response.length()) + "\r\n";
    header += "\r\n";

    send(clientSocket, header.c_str(), header.length(), 0);
    send(clientSocket, response.c_str(), response.length(), 0);

#ifdef _WIN32
    closesocket(clientSocket);
#else
    close(clientSocket);
#endif
  }

  void changeContext(const std::string& htmlFilePath, const std::string& cssFilePath)
  {
    htmlFile_ = readFile(htmlFilePath);
    cssFile_ = readFile(cssFilePath);
  }
private:
  void serverLoop()
  {
    while (running_)
    {
      {
        std::unique_lock<std::mutex> lock(pauseMutex_);
        pauseCondition_.wait(lock, [this] { return !paused_ || !running_; });
      }

      if (!running_) break;

      clientAddressLength_ = sizeof(clientAddress_);
      int clientSocket = accept(serverSocket_, (struct sockaddr*)&clientAddress_, &clientAddressLength_);
      
      if (clientSocket < 0)
      {
        if (running_) {
          Output() << "Error accepting client connection." << '\n';
        }
        continue;
      }

      handleClient(clientSocket);
    }
  }
};

}

#endif  // SERVERCLIENT_HPP