#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "keycode_lookup.h"
#include "remap_operator.h"

// Utility functions.

std::vector<std::string> split_string(const std::string& str, char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(str);
  while (std::getline(tokenStream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

std::string removeComment(const std::string& line) {
  auto pos = line.find("//");
  if (pos != std::string::npos) {
    return line.substr(0, pos);
  }
  return line;
}

std::string trim(const std::string& line) {
  static auto trim_chars = " \t\n\r\f\v";
  auto start = line.find_first_not_of(trim_chars);
  if (start == std::string::npos) {
    return "";
  }
  auto end = line.find_last_not_of(trim_chars);
  return line.substr(start, end - start + 1);
}

// Main class.

class ConfigParser {
 public:
  [[nodiscard]] bool parse(const std::vector<std::string>& lines) {
    bool success = true;
    for (const auto& line : lines) {
      success &= parse_line(line);
    }
    return success;
  }

  const Remapper getRemapper() { return remapper_; }

 private:
  [[nodiscard]] bool parse_line(const std::string& original_line) {
    // Ignore comments and empty lines.
    std::string line = trim(removeComment(original_line));
    if (line.empty()) {
      return true;
    }

    // Split the config line into the key combination and the action.
    auto parts = split_string(line, '=');
    if (parts.size() != 2) {
      std::cerr << "ERROR Invalid line - " << original_line << std::endl;
      return false;
    }

    std::string key_combo = trim(parts[0]);
    std::string action = trim(parts[1]);

    // Split key combination by '+', e.g., "DEL + END"
    auto keys = split_string(key_combo, '+');
    if (keys.size() == 1) {
      // TODO: Add mapping from left code to right code
    } else if (keys.size() == 2) {
      // TODO: Add mapping into a layer.
      // TODO: Consider special case of "*".
    } else {
      std::cerr << "ERROR Cannot have more than 1 '+' in line:" << original_line
                << std::endl;
      return false;
    }
    return true;
  }

  Remapper remapper_;
};

int main() {
  ConfigParser config_parser;

  // Example config lines to be parsed.
  std::vector<std::string> config_lines = {
      "CAPSLOCK + 1 = F1",         "CAPSLOCK + 2 = F2",
      "^RIGHT_CTRL = ^RIGHT_CTRL", "RIGHT_CTRL + 1 = ~RIGHT_CTRL + F1",
      "RIGHT_CTRL + * = *",        "^SHIFT = ^SHIFT",
      "SHIFT + ESC = BACKTICK",    "DEL + END = VOLUME_UP",
      "DEL + nothing = DEL"};

  // Parse the config and set up the remapper.
  if (!config_parser.parse(config_lines)) {
    exit(-1);
  }

  return 0;
}
