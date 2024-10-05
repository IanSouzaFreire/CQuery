#ifndef CQUERY_CORE_INCLUDES_HPP
#define CQUERY_CORE_INCLUDES_HPP

#include <condition_variable>
#include <unordered_map>
#include <string_view>
#include <functional>
#include <typeindex>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <string>
#include <memory>
#include <vector>
#include <cerrno>
#include <thread>
#include <atomic>
#include <mutex>
#include <array>

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