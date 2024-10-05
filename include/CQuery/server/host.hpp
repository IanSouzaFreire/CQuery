#ifndef CQUERY_SERVER_HOST_HPP
#define CQUERY_SERVER_HOST_HPP

#ifdef _WIN32
#include <CQuery/platform/windows/host.hpp>
#else
#include <CQuery/platform/linux/host.hpp>
#endif

namespace CQuery {

class Element;
class _$;

class ServerClient {
  friend class _$;
  std::unordered_map<std::string, std::any> userProcs_;
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
  // Platform specific
  ServerClient(int port) : paused_(false);

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
      contentType = "text/css";
      response = cssFile_;
    } else {
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

  // Platform specific
  void openInBrowser();

  template <typename F>
  void userProc(const std::string& name, F&& func)
  {
    userProcs_[name] = std::forward<F>(func);
  }

  template <typename F>
  F& getUserProc(const std::string& name)
  {
    return std::any_cast<F&>(userProcs_[name]);
  }

  template <typename F>
  auto executeUserProc(const std::string& name)
  {
    return std::any_cast<F&>(userProcs_[name])(running_, clientAddress_, serverSocket_);
  }

  template <typename F, typename... Args>
  auto executeUserProc(const std::string& name, Args&&... args)
  {
    return std::any_cast<F&>(userProcs_[name])(std::forward<Args>(args)...);
  }

  template <typename F, typename... Args>
  auto executeUserProcWithState(const std::string& name, Args&&... args)
  {
    return std::any_cast<F&>(userProcs_[name])(running_, clientAddress_, serverSocket_, std::forward<Args>(args)...);
  }

  bool hasUserProc(const std::string& name) const
  {
    return userProcs_.find(name) != userProcs_.end();
  }

  void removeUserProc(const std::string& name)
  {
    userProcs_.erase(name);
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

      try {
        if (hasUserProc("serverLoop")) {
          executeUserProcWithState<void(std::atomic<bool>&, sockaddr_in&, int)>("serverLoop");
        } else {
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
      } catch (const std::exception& e) {
        Output() << "Exception in server loop: " << e.what() << '\n';
      } catch (...) {
        Output() << "Unknown exception in server loop\n";
      }
    }
  }

  int getServerSocket() const {
    return serverSocket_;
  }

  const sockaddr_in& getClientAddress() const {
    return clientAddress_;
  }

  #ifdef _WIN32
  int getClientAddressLength() const {
  #else
  socklen_t getClientAddressLength() const {
  #endif
    return clientAddressLength_;
  }
};

}

#endif