#ifndef WIN32_HOST_HPP
#define WIN32_HOST_HPP

namespace CQuery {

void initWinsock()
{
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
  {
    throw std::runtime_error("Error initializing Winsock.");
  }
}

ServerClient::ServerClient(int port) : paused_(false)
{
  initWinsock();
  serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);

  if (serverSocket_ < 0)
  {
    throw std::runtime_error("Error creating server socket: " + std::to_string(WSAGetLastError()));
  }

  serverAddress_.sin_family = AF_INET;
  serverAddress_.sin_port = htons(port);
  serverAddress_.sin_addr.s_addr = INADDR_ANY;

  // Allow reuse of the address
  int opt = 1;
  if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) < 0)
  {
    throw std::runtime_error("Error setting socket option: " + std::to_string(WSAGetLastError()));
  }

    if (bind(serverSocket_, (struct sockaddr*)&serverAddress_, sizeof(serverAddress_)) < 0)
    {
      throw std::runtime_error("Error binding server socket to address: " + std::to_string(WSAGetLastError()));
    }

    if (listen(serverSocket_, 3) < 0)
    {
      throw std::runtime_error("Error listening on server socket: " + std::to_string(WSAGetLastError()));
    }

    running_ = false;
  }

void ServerClient::openInBrowser()
{
  std::string url = "http://localhost:" + std::to_string(ntohs(serverAddress_.sin_port));

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
}

}

#endif