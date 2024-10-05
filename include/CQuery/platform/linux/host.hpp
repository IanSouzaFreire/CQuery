#ifndef LINUX_HOST_HPP
#define LINUX_HOST_HPP

namespace CQuery {

ServerClient::ServerClient(int port) : paused_(false)
{
  serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);

  if (serverSocket_ < 0)
  {
    throw std::runtime_error("Error creating server socket: " + std::string(strerror(errno)));
  }

  serverAddress_.sin_family = AF_INET;
  serverAddress_.sin_port = htons(port);
  serverAddress_.sin_addr.s_addr = INADDR_ANY;

  // Allow reuse of the address
  int opt = 1;
  if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) < 0)
  {
    throw std::runtime_error("Error setting socket option: " + std::string(strerror(errno)));
  }

  if (bind(serverSocket_, (struct sockaddr*)&serverAddress_, sizeof(serverAddress_)) < 0)
  {
    throw std::runtime_error("Error binding server socket to address: " + std::string(strerror(errno)));
  }

  if (listen(serverSocket_, 3) < 0)
  {
    throw std::runtime_error("Error listening on server socket: " + std::string(strerror(errno)));
  }

  running_ = false;
}

void ServerClient::openInBrowser()
{
  std::string url = "http://localhost:" + std::to_string(ntohs(serverAddress_.sin_port));

  std::string cmd = "xdg-open " + url + " || open " + url + " || echo 'Unable to open browser automatically'";
  int result = system(cmd.c_str());
  if (result == 0) {
    Output() << "Opened " << url << " in the default browser.\n";
  } else {
    Output() << "Failed to open browser. Please open " << url << " manually.\n";
  }
}

}

#endif