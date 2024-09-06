# CQuery

## A JQuery inspired single header library for C++

Using only the C++ standard library, I wanted to create a simple and easy to use library for creating web servers to impress my friends, classmates, and family. I have found the experience with JavaScript great, mainly with the JQuery library. And so, I tried creating something similar.

## Why?

I love JQuery, I love C++, why not have both?

## Usage

```cpp
#include <iostream>
#include <filesystem>
#include "CQuery.hpp"

int main(const int, const char**)
{
  using namespace CQuery;

  OutputMethod() = OUT_METHOD::SILENT;

  int available_port = findAvailablePort(8080, 9000);
  
  if (available_port == -1) {
    Output() << "No available ports found between 8080 and 9000." << '\n';
    return 1;
  }

  Output() << "Using port: " << available_port << '\n';

  std::filesystem::path executablePath = std::filesystem::current_path();
  std::string htmlPath = (executablePath / "src" / "index.html").string();
  std::string cssPath = (executablePath / "src" / "style.css").string();

  ServerClient server(available_port);

  try {
    server.changeContext(htmlPath, cssPath);
    server.startServer();
  } catch (const std::exception& e) {
    Output() << "An error occurred: " << e.what() << '\n';
    return 1;
  }

  while (true) {}

  server.closeServer();
  return 0;
}
```

## Features

- Not yet implemented

## Contributing

Feel free to contribute to this project with whatever you find relevant to it.
