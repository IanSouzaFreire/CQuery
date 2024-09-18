#ifndef CQUERY_UTIL_OUT_HPP
#define CQUERY_UTIL_OUT_HPP

namespace CQuery {

enum class OUT_METHOD {
  _SET_,
  _OUT_,
  CMD,
  FILE,
  SILENT
};

OUT_METHOD& OutputMethod() {
  static OUT_METHOD current = OUT_METHOD::CMD;
  return current;
}

std::ostream& Output() {
  switch (OutputMethod()) {
    case OUT_METHOD::CMD:
      return std::cout;
    case OUT_METHOD::FILE:
      static std::ofstream fileStream("h.log");
      return fileStream;
    case OUT_METHOD::SILENT:
      static std::ostream nullStream(nullptr);
      return nullStream;
    default:
      return std::cout;
  }
}

}

#endif