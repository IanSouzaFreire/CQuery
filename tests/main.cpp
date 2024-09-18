#include <catch2/catch_test_macros.hpp>
#include <CQuery.hpp>
#include <sstream>

// Fibonacci function to test cache
int fib(int n)
{
  if (n <= 1)
    return n;
  return fib(n - 1) + fib(n - 2);
}

TEST_CASE("CQuery Cache class functionality", "[cache]")
{
  CQuery::Cache<int, int> fibonacci(fib);

  SECTION("Cache functionality")
  {
    REQUIRE(fibonacci.cache(5) == 5);
    REQUIRE(fibonacci.cache(5) == 5);
    REQUIRE(fibonacci.cache(10) == 55);
  }

  SECTION("Cache size")
  {
    REQUIRE(fibonacci.cache_size() == 2);
  }

  SECTION("Cache hits and misses")
  {
    REQUIRE(fibonacci.cache_hits() == 3);
    REQUIRE(fibonacci.cache_misses() == 0);
  }

  SECTION("Cache clear")
  {
    fibonacci.cache_clear();
    REQUIRE(fibonacci.cache_size() == 0);
  }
}

TEST_CASE("CQuery Typed cache class functionality", "[typed-cache]")
{
  CQuery::TypedCache<int, int> fibonacci(fib);

  SECTION("Typed cache functionality")
  {
    REQUIRE(fibonacci.cache(5) == 5);
    REQUIRE(fibonacci.cache(10) == 55);
  }

  SECTION("Typed cache size")
  {
    REQUIRE(fibonacci.cache_size() == 2);
  }

  SECTION("Typed cache hits and misses")
  {
    REQUIRE(fibonacci.cache_hits() == 2);
    REQUIRE(fibonacci.cache_misses() == 0);
  }

  SECTION("Typed cache clear")
  {
    fibonacci.cache_clear();
    REQUIRE(fibonacci.cache_size() == 0);
  }
}

TEST_CASE("CQuery Element class functionality", "[element]")
{
  CQuery::Element elem("div", "content");

  SECTION("Element initialization")
  {
    REQUIRE(elem.tag == "div");
    REQUIRE(elem.content == "content");
    REQUIRE(elem.children.empty());
    REQUIRE(elem.attributes.empty());
  }

  SECTION("Setting attributes")
  {
    elem.attr("class", "test-class");
    REQUIRE(elem.attributes["class"] == "test-class");
  }

  SECTION("Appending child elements")
  {
    CQuery::Element child("span", "child content");
    elem.append(child);
    REQUIRE(elem.children.size() == 1);
    REQUIRE(elem.children[0].tag == "span");
    REQUIRE(elem.children[0].content == "child content");
  }

  SECTION("Setting text content")
  {
    elem.text("new content");
    REQUIRE(elem.content == "new content");
  }

  SECTION("Converting element to string")
  {
    elem.attr("id", "test-id");
    elem.append(CQuery::Element("p", "paragraph"));
    std::string expected = "<div id=\"test-id\">content<p>paragraph</p></div>";
    REQUIRE(elem.toString() == expected);
  }
}

TEST_CASE("CQuery _$ class functionality", "[jquery]")
{
  CQuery::_$ query;

  SECTION("Selector functionality")
  {
    query("#test-id");
    REQUIRE(query.html() == "<div id=\"test-id\"></div>");

    query(".test-class");
    REQUIRE(query.html() == "<div class=\"test-class\"></div>");

    query("span");
    REQUIRE(query.html() == "<span></span>");
  }

  SECTION("Attribute manipulation")
  {
    query("div").attr("data-test", "value");
    REQUIRE(query.html() == "<div data-test=\"value\"></div>");
  }

  SECTION("Appending elements")
  {
    CQuery::Element child("p", "text");
    query("div").append(child);
    REQUIRE(query.html() == "<div><p>text</p></div>");
  }

  SECTION("Setting text content")
  {
    query("div").text("new content");
    REQUIRE(query.html() == "<div>new content</div>");
  }
}

TEST_CASE("CQuery ServerClient class functionality", "[server]")
{
  int port = CQuery::findAvailablePort(8000, 9000);
  REQUIRE(port != -1);

  CQuery::ServerClient server(port);

  SECTION("Server initialization")
  {
    REQUIRE_NOTHROW(server.startServer());
    server.closeServer();
  }

  SECTION("Server pause and resume")
  {
    server.startServer();
    REQUIRE_NOTHROW(server.pauseServer());
    REQUIRE_NOTHROW(server.resumeServer());
    server.closeServer();
  }

  SECTION("Changing server context")
  {
    // Create temporary HTML and CSS files for testing
    std::ofstream htmlFile("test.html");
    htmlFile << "<html><body>Test</body></html>";
    htmlFile.close();

    std::ofstream cssFile("test.css");
    cssFile << "body { color: red; }";
    cssFile.close();

    REQUIRE_NOTHROW(server.changeContext("test.html", "test.css"));

    // Clean up temporary files
    std::remove("test.html");
    std::remove("test.css");
  }
}

TEST_CASE("CQuery utility functions", "[util]")
{
  SECTION("proc function")
  {
    auto lambda = []() { return 42; };
    REQUIRE(CQuery::proc(lambda) == 42);

    int value = 10;
    auto capturingLambda = [&value]() { return value * 2; };
    REQUIRE(CQuery::proc(capturingLambda) == 20);

    REQUIRE(CQuery::proc([=]() { return value * 2; }) == 20);
  }

  SECTION("findAvailablePort function")
  {
    int port = CQuery::findAvailablePort(8000, 9000);
    REQUIRE(port >= 8000);
    REQUIRE(port <= 9000);
  }
}

TEST_CASE("CQuery Output functionality", "[output]")
{
  SECTION("Default output method")
  {
    REQUIRE(CQuery::OutputMethod() == CQuery::OUT_METHOD::CMD);
  }

  SECTION("Changing output method")
  {
    CQuery::OutputMethod() = CQuery::OUT_METHOD::SILENT;
    REQUIRE(CQuery::OutputMethod() == CQuery::OUT_METHOD::SILENT);

    // Reset to default for other tests
    CQuery::OutputMethod() = CQuery::OUT_METHOD::CMD;
  }

  SECTION("Output to string")
  {
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

    CQuery::Output() << "Test output";

    std::cout.rdbuf(old);

    REQUIRE(buffer.str() == "Test output");
  }
}