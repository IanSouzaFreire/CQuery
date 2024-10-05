#ifndef CQUERY_SERVER_HANDLER_HPP
#define CQUERY_SERVER_HANDLER_HPP

namespace CQuery {

class Element;
class _$;
class ServerClient;

class Element {
public:
  std::string tag;
  std::string content;
  std::vector<Element> children;
  std::unordered_map<std::string, std::string> attributes;

  Element(const std::string& t = "", const std::string& c = "") : tag(t), content(c) {}

  Element& attr(const std::string& key, const std::string& value) {
    attributes[key] = value;
    return *this;
  }

  Element& append(const Element& child) {
    children.push_back(child);
    return *this;
  }

  Element& text(const std::string& text) {
    content = text;
    return *this;
  }

  std::string toString() const {
    std::stringstream ss;
    ss << "<" << tag;
    for (const auto& attr : attributes) {
      ss << " " << attr.first << "=\"" << attr.second << "\"";
    }
    ss << ">";
    ss << content;
    for (const auto& child : children) {
      ss << child.toString();
    }
    ss << "</" << tag << ">";
    return ss.str();
  }
};

class _$ {
  friend class ServerClient;
  ServerClient* Handler;
  std::vector<Element> elements;

public:
  _$() : Handler(nullptr) {}

  void operator()(ServerClient& handler) {
    Handler = &handler;
  }

  _$& operator()(const std::string& selector) {
    elements.clear();
    if (Handler) {
      if (selector[0] == '#') {
        elements.push_back(Element("div").attr("id", selector.substr(1)));
      } else if (selector[0] == '.') {
        elements.push_back(Element("div").attr("class", selector.substr(1)));
      } else {
        elements.push_back(Element(selector));
      }
    }
    return *this;
  }

  _$& attr(const std::string& key, const std::string& value) {
    for (auto& elem : elements) {
      elem.attr(key, value);
    }
    return *this;
  }

  _$& append(const Element& child) {
    for (auto& elem : elements) {
      elem.append(child);
    }
    return *this;
  }

  _$& text(const std::string& content) {
    for (auto& elem : elements) {
      elem.content = content;
    }
    return *this;
  }

  std::string html() const {
    std::stringstream ss;
    for (const auto& elem : elements) {
      ss << elem.toString();
    }
    return ss.str();
  }
};

}

#endif