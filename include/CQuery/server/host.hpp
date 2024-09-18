#ifndef CQUERY_SERVER_HOST_HPP
#define CQUERY_SERVER_HOST_HPP

namespace CQuery {

#ifdef _WIN32
// Initialize Winsock for Windows
void initWinsock()
{
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
  {
    throw std::runtime_error("Error initializing Winsock.");
  }
}
#endif

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

  // Read file content
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
  // Constructor
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
    serverAddress_.sin_addr.s_addr = INADDR_ANY;

    // Allow reuse of the address
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

  // Destructor
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

  // Start the server
  void startServer()
  {
    running_ = true;
    serverThread_ = std::thread(&ServerClient::serverLoop, this);
    Output() << "Server started. Listening for incoming connections..." << '\n';
  }

  // Close the server
  void closeServer()
  {
    running_ = false;
    if (serverThread_.joinable()) {
      serverThread_.join();
    }
    Output() << "Server stopped." << '\n';
  }

  // Pause the server
  void pauseServer()
  {
    paused_ = true;
    Output() << "Server paused." << '\n';
  }

  // Resume the server
  void resumeServer()
  {
    paused_ = false;
    pauseCondition_.notify_all();
    Output() << "Server resumed." << '\n';
  }

  // Handle client connection
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

  // Change the context (HTML and CSS content)
  void changeContext(const std::string& htmlFilePath, const std::string& cssFilePath)
  {
    htmlFile_ = readFile(htmlFilePath);
    cssFile_ = readFile(cssFilePath);
  }

  // Open the server URL in the default browser
  void openInBrowser()
  {
    std::string url = "http://localhost:" + std::to_string(ntohs(serverAddress_.sin_port));

#ifdef _WIN32
    // Windows-specific code
    SHELLEXECUTEINFOA shellExecuteInfo = {0};
    shellExecuteInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
    shellExecuteInfo.fMask = SEE_MASK_NOASYNC;
    shellExecuteInfo.lpVerb = "open";
    shellExecuteInfo.lpFile = url.c_str();
    shellExecuteInfo.nShow = SW_SHOWNORMAL;

    if (ShellExecuteExA(&shellExecuteInfo)) {
      Output() << "Opened " << url << " in the default browser.\n";
    } else {
      DWORD error = GetLastError();
      Output() << "Failed to open browser. Error code: " << error << "\n";
    }
#else
    // Unix-like systems (Linux, macOS)
    std::string cmd = "xdg-open " + url + " || open " + url + " || echo 'Unable to open browser automatically'";
    int result = system(cmd.c_str());
    if (result == 0) {
      Output() << "Opened " << url << " in the default browser.\n";
    } else {
      Output() << "Failed to open browser. Please open " << url << " manually.\n";
    }
#endif
  }

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
  // Main server loop
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
          // Call user-defined serverLoop function if it exists
          executeUserProcWithState<void(std::atomic<bool>&, sockaddr_in&, int)>("serverLoop");
        } else {
          // Default server loop behavior
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

  // Add a method to get the server socket
  int getServerSocket() const {
    return serverSocket_;
  }

  // Add a method to get the client address
  const sockaddr_in& getClientAddress() const {
    return clientAddress_;
  }

  // Add a method to get the client address length
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