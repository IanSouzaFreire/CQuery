#ifndef INCLUDES_HPP
#define INCLUDES_HPP

#include <unordered_map>
#include <sstream>
#include <condition_variable>
#include <mutex>
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

// Platform-specific includes
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <Ws2tcpip.h>
#include <shellapi.h>
#include <unistd.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#endif