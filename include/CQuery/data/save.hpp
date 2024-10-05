#ifndef CQUERY_DATA_SAVE_HPP
#define CQUERY_DATA_SAVE_HPP

namespace CQuery {

constexpr std::array<char, 256> generateKey()
{
  std::array<char, 256> key{};

  for (size_t i = 0; i < key.size(); ++i)
  {
    key[i] = static_cast<char>(i);
  }

  std::reverse(key.begin(), key.end());
  return key;
}

constexpr std::string_view encrypt(const char input[])
{
  constexpr auto key = generateKey();
  std::string encrypted;

  for (size_t i = 0; input[i] != '\0'; ++i)
  {
    encrypted += key[static_cast<unsigned char>(input[i])];
  }

  return std::string_view(encrypted);
}

constexpr std::string_view decrypt(const std::string_view input)
{
  constexpr auto key = generateKey();
  std::string decrypted;

  for (char c : input)
  {
    auto it = std::find(key.begin(), key.end(), c);

    if (it != key.end()) {
      decrypted += static_cast<char>(std::distance(key.begin(), it));
    }
  }

  return std::string_view(decrypted);
}

class CookieFile {
private:
  std::unordered_map<std::string, std::unordered_map<std::string, std::unique_ptr<void>>> data_;
  std::string filepath_;

  template <typename T>
  std::string serialize(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
  }

  template <typename T>
  T deserialize(const std::string& str) {
    T value;
    std::istringstream iss(str);
    iss >> value;
    return value;
  }

  void encryptAndSave() {
    std::ofstream file(filepath_, std::ios::binary);
    if (!file) {
      throw std::runtime_error("Could not open file for writing: " + filepath_);
    }

    for (const auto& row : data_) {
      for (const auto& column : row.second) {
        std::string value = serialize(*static_cast<std::decay_t<decltype(*column.second)>>>(column.second.get());
        std::string encryptedValue = encrypt(value.c_str());
        file << row.first << "," << column.first << "," << encryptedValue << "\n";
      }
    }
    file.close();
  }

  void loadAndDecrypt() {
    std::ifstream file(filepath_, std::ios::binary);
    if (!file) {
      throw std::runtime_error("Could not open file for reading: " + filepath_);
    }

    std::string line;
    while (std::getline(file, line)) {
      std::istringstream ss(line);
      std::string row, column, encryptedValue;
      if (std::getline(ss, row, ',') && std::getline(ss, column, ',') && std::getline(ss, encryptedValue)) {
        std::string decryptedValue = decrypt(encryptedValue);
        // Use a type-erased approach to store different types
        data_[row][column] = std::make_unique<std::string>(decryptedValue);
      }
    }
    file.close();
  }

public:
  CookieFile(std::string_view filepath) : filepath_(filepath) {
    loadAndDecrypt();
  }

  ~CookieFile() {
    encryptAndSave();
  }

  template <typename T>
  void create(const std::string& row, const std::string& column, T value) {
    auto rowIt = data_.find(row);
    if (rowIt != data_.end()) {
      auto colIt = rowIt->second.find(column);
      if (colIt != rowIt->second.end()) {
        throw std::runtime_error("Cannot create data; the specified cell is not empty.");
      }
    }
    data_[row][column] = std::make_unique<std::decay_t<T>>(value);
  }

  template <typename T>
  T read(const std::string& row, const std::string& column) {
    auto rowIt = data_.find(row);
    if (rowIt != data_.end()) {
      auto colIt = rowIt->second.find(column);
      if (colIt != rowIt->second.end()) {
        return deserialize<T>(*static_cast<std::string*>(colIt->second.get()));
      }
    }
    throw std::runtime_error("Data not found");
  }

  template <typename T>
  void update(const std::string& row, const std::string& column, T value) {
    data_[row][column] = std::make_unique<std::decay_t<T>>(value);
  }

  void remove(const std::string& row, const std::string& column) {
    auto rowIt = data_.find(row);
    if (rowIt != data_.end()) {
      rowIt->second.erase(column);
    }
  }
};

}

#endif