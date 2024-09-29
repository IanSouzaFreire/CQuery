<div align="center">
  <img src="public/images/Frame 1.svg">
</div>

# CQuery

## A JQuery inspired library for C++

Using only the C++ standard library, I wanted to create a simple and easy to use library for creating web servers to impress my friends, classmates, and family. I have found the experience with JavaScript great, mainly with the JQuery library. And so, I tried creating something similar.

## Why?

I love JQuery, I love C++, why not have both?

## Information

This library uses catchorgs's [**Catch2**](https://github.com/catchorg/Catch2) for unit testing. To test the library, go to the "tests" directory and compile the code

## Usage

```cpp
#include <iostream>
#include <filesystem>
#include <CQuery.hpp> // Counting as a header in system path

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

Serves for things not related to servers too!

```cpp
// Counting as a headers in system path
#include <CQuery/util/out.hpp>
#include <CQuery/util/proc.hpp>

int main(int, char** argv)
{
  using namespace CQuery;

  if (argv[1] == "debug") {
    OutputMethod() = OUT_METHOD::CMD;
  } else {
    OutputMethod() = OUT_METHOD::SILENT;
  }

  // Works for anyone that can't be bothered to use conditional compilation
  Output() << "Hello World" << '\n';

  return 0;
}
```

## Contributing

Feel free to contribute to this project with whatever you find relevant to it. More information in the [**documentation**](https://github.com/IanSouzaFreire/CQuery/tree/main/docs/).
