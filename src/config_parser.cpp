#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "keycode_lookup.h"
#include "remap_operator.h"

using std::string;

// Utility functions.

std::vector<string> split_string(const string& str, char delimiter) {
  std::vector<string> tokens;
  string token;
  std::istringstream tokenStream(str);
  while (std::getline(tokenStream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

string removeComment(const string& line) {
  auto pos = line.find("//");
  if (pos != string::npos) {
    return line.substr(0, pos);
  }
  return line;
}

string trim(const string& line) {
  static auto trim_chars = " \t\n\r\f\v";
  auto start = line.find_first_not_of(trim_chars);
  if (start == string::npos) {
    return "";
  }
  auto end = line.find_last_not_of(trim_chars);
  return line.substr(start, end - start + 1);
}

std::pair<char, std::optional<int>> split_key_prefix(string name) {
  static const string prefixes = "~^";
  char prefix = 0;
  if (prefixes.find(name[0]) != std::string::npos) {
    prefix = name[0];
    name = name.substr(1);
  }
  return {prefix, nameToKeyCode("KEY_" + name)};
}

// Main class.

class ConfigParser {
 public:
  ConfigParser(Remapper* remapper) { remapper_ = remapper; }
  [[nodiscard]] bool parse(const std::vector<string>& lines) {
    bool success = true;
    for (const auto& line : lines) {
      success &= parse_line(line);
    }
    return success;
  }

 private:
  bool parse_assignment(const string& left, string& right) {
    auto [left_prefix, left_key] = split_key_prefix(left);
    if (!left_key.has_value()) return false;

    std::vector<Action> actions;
    std::istringstream iss(right);
    string token;
    while (iss >> token) {
      auto [right_prefix, right_key] = split_key_prefix(token);
      if (!right_key.has_value()) return false;
      if (right_prefix == 0 || right_prefix == '^') {
        actions.push_back(KeyPressEvent(*right_key));
      }
      if (right_prefix == 0 || right_prefix == '~') {
        actions.push_back(KeyReleaseEvent(*right_key));
      }
    }

    remapper_->add_mapping("",
                           left_prefix == '~' ? KeyReleaseEvent(*left_key)
                                              : KeyPressEvent(*left_key),
                           actions);
    return true;
  }

  [[nodiscard]] bool parse_line(const string& original_line) {
    // Ignore comments and empty lines.
    string line = trim(removeComment(original_line));
    if (line.empty()) {
      return true;
    }

    // Split the config line into the key combination and the action.
    auto parts = split_string(line, '=');
    if (parts.size() != 2) {
      std::cerr << "ERROR Invalid line - " << original_line << std::endl;
      return false;
    }

    string key_combo = trim(parts[0]);
    string action = trim(parts[1]);

    // Split key combination by '+', e.g., "DEL + END"
    auto keys = split_string(key_combo, '+');
    if (keys.size() == 1) {
      // TODO: Add mapping from left code to right code
      parse_assignment(key_combo, action);
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

  Remapper* remapper_;
};

int main() {
  Remapper remapper;
  ConfigParser config_parser(&remapper);

  // Example config lines to be parsed.
  std::vector<string> config_lines = {
      "CAPSLOCK + 1 = F1",          "CAPSLOCK + 2 = F2",
      "^RIGHTCTRL = ^RIGHTCTRL",    "RIGHTCTRL + 1 = ~RIGHTCTRL F1",
      "RIGHTCTRL + * = *",          "^LEFTSHIFT = ^LEFTSHIFT",
      "LEFTSHIFT + ESC = BACKTICK", "DEL + END = VOLUME_UP",
      "DEL + nothing = DEL",        "^A = ~D ^A"};

  // Parse the config and set up the remapper.
  if (!config_parser.parse(config_lines)) {
    exit(-1);
  }

  remapper.dump_config();

  return 0;
}
