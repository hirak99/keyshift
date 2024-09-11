#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "keycode_lookup.h"
#include "remap_operator.h"

using std::string;

const string kDefaultLayerName = "";

// Utility functions.

std::vector<string> SplitString(const string& str, char delimiter) {
  std::vector<string> tokens;
  string token;
  std::istringstream tokenStream(str);
  while (std::getline(tokenStream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

string RemoveComment(const string& line) {
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

std::pair<char, std::optional<int>> SplitKeyPrefix(string name) {
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
  // Converts a string like "~D ^A" to actions.
  std::vector<Action> AssignmentToActions(const string& assignment) {
    std::vector<Action> actions;
    std::istringstream iss(assignment);
    string token;
    while (iss >> token) {
      const auto [right_prefix, right_key] = SplitKeyPrefix(token);
      if (!right_key.has_value()) {
        throw std::invalid_argument("Invalid keycode.");
      };
      if (right_prefix == 0 || right_prefix == '^') {
        actions.push_back(KeyPressEvent(*right_key));
      }
      if (right_prefix == 0 || right_prefix == '~') {
        actions.push_back(KeyReleaseEvent(*right_key));
      }
    }
    return actions;
  }

  bool ParseAssignment(const string& layer_name, const string& key_str,
                       const string& assignment) {
    const auto [left_prefix, left_key] = SplitKeyPrefix(key_str);
    if (!left_key.has_value()) return false;

    std::vector<Action> actions;
    try {
      actions = AssignmentToActions(assignment);
    } catch (const std::invalid_argument&) {
      return false;
    }

    remapper_->AddMapping(layer_name,
                          left_prefix == '~' ? KeyReleaseEvent(*left_key)
                                             : KeyPressEvent(*left_key),
                          actions);
    return true;
  }

  bool ParseLayerAssignment(const string& layer_key_str, const string& key_str,
                            const string& assignment) {
    const auto [layer_prefix, layer_key] = SplitKeyPrefix(layer_key_str);
    if (layer_prefix != 0) {
      perror("ERROR: Prefix (^ or ~) for layer keys is not supported yet.");
      return false;
    }
    string layer_name = layer_key_str + "_layer";

    // Add default to layer mapping.
    remapper_->AddMapping(kDefaultLayerName, KeyReleaseEvent(*layer_key),
                          {remapper_->ActionActivateState(layer_name)});

    // Handle SHIFT + * = *.
    if (key_str == "*") {
      if (assignment != "*") {
        perror("ERROR: Must be a * on the right side of for KEY + * = *");
        return false;
      }
      remapper_->SetAllowOtherKeys(layer_name, true);
      return true;
    }

    // Handle DELETE + nothing = DELETE.
    if (key_str == "nothing") {
      try {
        remapper_->SetNullEventActions(layer_name,
                                       AssignmentToActions(assignment));
      } catch (const std::invalid_argument&) {
        return false;
      }
      return true;
    }

    return ParseAssignment(layer_name, key_str, assignment);
  }

  [[nodiscard]] bool parse_line(const string& original_line) {
    // Ignore comments and empty lines.
    string line = trim(RemoveComment(original_line));
    if (line.empty()) {
      return true;
    }

    // Split the config line into the key combination and the action.
    auto parts = SplitString(line, '=');
    if (parts.size() != 2) {
      std::cerr << "ERROR Invalid line - " << original_line << std::endl;
      return false;
    }

    string key_combo = trim(parts[0]);
    string action = trim(parts[1]);

    // Split key combination by '+', e.g., "DEL + END"
    auto keys = SplitString(key_combo, '+');
    if (keys.size() == 1) {
      return ParseAssignment(kDefaultLayerName, trim(keys[0]), action);
    } else if (keys.size() == 2) {
      return ParseLayerAssignment(trim(keys[0]), trim(keys[1]), action);
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
      "CAPSLOCK + 1 = F1",         "CAPSLOCK + 2 = F2",
      "^RIGHTCTRL = ^RIGHTCTRL",   "RIGHTCTRL + 1 = ~RIGHTCTRL F1",
      "RIGHTCTRL + * = *",         "^LEFTSHIFT = ^LEFTSHIFT",
      "LEFTSHIFT + ESC = GRAVE",   "DELETE + END = VOLUMEUP",
      "DELETE + nothing = DELETE", "^A = ~D ^A"};

  // Parse the config and set up the remapper.
  if (!config_parser.parse(config_lines)) {
    perror("Could not parse config, exiting.\n");
    exit(-1);
  }

  remapper.DumpConfig();

  return 0;
}
