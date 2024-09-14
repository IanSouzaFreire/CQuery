#ifndef PORT_HPP
#define PORT_HPP

namespace CQuery {

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
      continue;
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

}

#endif