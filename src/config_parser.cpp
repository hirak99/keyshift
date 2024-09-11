#include "config_parser.h"

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
  std::istringstream token_stream(str);
  while (std::getline(token_stream, token, delimiter)) {
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

string StringTrim(const string& line) {
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

ConfigParser::ConfigParser(Remapper* remapper) { remapper_ = remapper; }

[[nodiscard]] bool ConfigParser::Parse(const std::vector<string>& lines) {
  bool success = true;
  for (const auto& line : lines) {
    success &= ParseLine(line);
  }
  return success;
}

// PRIVATE

// Converts a string like "~D ^A" to actions.
std::vector<Action> ConfigParser::AssignmentToActions(
    const string& assignment) {
  const auto tokens = SplitString(assignment, ' ');
  return AssignmentToActions(tokens);
}

std::vector<Action> ConfigParser::AssignmentToActions(
    const std::vector<string>& tokens) {
  std::vector<Action> actions;

  for (const auto& token : tokens) {
    const auto [right_prefix, right_key] = SplitKeyPrefix(token);
    if (!right_key.has_value()) {
      std::cerr << "ERROR: Invalid keycode. Line - " << line_being_parsed_
                << std::endl;
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

bool ConfigParser::ParseAssignment(const string& layer_name,
                                   const string& key_str,
                                   const string& assignment) {
  const auto [left_prefix, left_key] = SplitKeyPrefix(key_str);
  if (!left_key.has_value()) return false;

  std::vector<Action> actions;
  try {
    std::vector<string> tokens = SplitString(assignment, ' ');
    // For assignments like A = B, convert to [^A = ^B, ~A = ~B].
    if (left_prefix == 0) {
      int n_tokens = tokens.size();
      string last_token = tokens[n_tokens - 1];
      if (last_token[0] == '^' || last_token[0] == '~') {
        std::cerr << "ERROR: If left does not have a prefix (^ or ~), the "
                     "last token of assignment must not have either; line "
                  << line_being_parsed_ << std::endl;
        return false;
      }
      // On activation, do everything, but only activate the final key.
      tokens[n_tokens - 1] = "^" + last_token;
      remapper_->AddMapping(layer_name, KeyPressEvent(*left_key),
                            AssignmentToActions(tokens));
      // On release, do nothing, and only release the final key.
      remapper_->AddMapping(layer_name, KeyReleaseEvent(*left_key),
                            AssignmentToActions({"~" + last_token}));
      return true;
    } else {
      remapper_->AddMapping(layer_name,
                            left_prefix == '~' ? KeyReleaseEvent(*left_key)
                                               : KeyPressEvent(*left_key),
                            AssignmentToActions(tokens));
      return true;
    }
  } catch (const std::invalid_argument&) {
    std::cerr << "ParseAssignment: Failed at line " << line_being_parsed_
              << std::endl;
    return false;
  }
}

bool ConfigParser::ParseLayerAssignment(const string& layer_key_str,
                                        const string& key_str,
                                        const string& assignment) {
  const auto [layer_prefix, layer_key] = SplitKeyPrefix(layer_key_str);
  if (layer_prefix != 0) {
    perror("ERROR: Prefix (^ or ~) for layer keys is not supported yet.");
    return false;
  }
  string layer_name = layer_key_str + "_layer";

  // Add default to layer mapping.
  if (known_layers_.find(layer_name) == known_layers_.end()) {
    try {
      remapper_->AddMapping(kDefaultLayerName, KeyPressEvent(*layer_key),
                            {remapper_->ActionActivateState(layer_name)});
    } catch (std::invalid_argument&) {
      std::cerr << "ParseLayerAssignment: Failed at line " << line_being_parsed_
                << std::endl;
      return false;
    }
    remapper_->SetAllowOtherKeys(layer_name, false);
    known_layers_.insert(layer_name);
  }

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

[[nodiscard]] bool ConfigParser::ParseLine(const string& original_line) {
  // Ignore comments and empty lines.
  line_being_parsed_ = original_line;
  string line = StringTrim(RemoveComment(original_line));
  if (line.empty()) {
    return true;
  }

  // Split the config line into the key combination and the action.
  auto parts = SplitString(line, '=');
  if (parts.size() != 2) {
    std::cerr << "ERROR Invalid line - " << original_line << std::endl;
    return false;
  }

  string key_combo = StringTrim(parts[0]);
  string action = StringTrim(parts[1]);

  // Split key combination by '+', e.g., "DEL + END"
  auto keys = SplitString(key_combo, '+');
  if (keys.size() == 1) {
    return ParseAssignment(kDefaultLayerName, StringTrim(keys[0]), action);
  } else if (keys.size() == 2) {
    return ParseLayerAssignment(StringTrim(keys[0]), StringTrim(keys[1]),
                                action);
  } else {
    std::cerr << "ERROR Cannot have more than 1 '+' in line:" << original_line
              << std::endl;
    return false;
  }
  return true;
}
