/*
 * Copyright (c) 2024 Nomen Aliud (aka Arnab Bose)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "config_parser.h"

#include <expected>
#include <format>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "keycode_lookup.h"
#include "remap_operator.h"
#include "utility/essentials.h"

using std::string;

const string kDefaultLayerName = "";

// Any wait larger than this will not be allowed.
const int kMaxWaitMs = 1000;

// When on left, sets a default assignment e.g. "DELETE + nothing = DELETE".
// When on right, blocks a key e.g. "DELETE = nothing".
const string kNothingToken = "nothing";

// Utility functions.

string RemoveComment(string line) {
  // Support both // or # as comment begin.
  for (const string comment_marker : {"//", "#"}) {
    auto pos = line.find(comment_marker);
    if (pos != string::npos) {
      line = line.substr(0, pos);
    }
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

struct PrefixedKey {
  // E.g. "^", "~", or empty.
  std::optional<char> prefix;
  // Keycode.
  int key;
};

// Splits a token like "^A", "~A" or "A" into prefix "^", "~", "" respectively,
// with the suffix as "A".
std::expected<PrefixedKey, std::string> SplitKeyPrefix(string name) {
  static const string prefixes = "~^";
  std::optional<char> prefix = std::nullopt;
  if (prefixes.find(name[0]) != std::string::npos) {
    prefix = name[0];
    name = name.substr(1);
  }
  const auto& keycode = StartsWith(name, "KEY_") ? NameToKeyCode(name)
                                                 : NameToKeyCode("KEY_" + name);
  if (!keycode.has_value()) {
    return std::unexpected(std::format("Unknown key code '{}'.", name));
  }
  return PrefixedKey{prefix, keycode.value()};
}

std::string LayerNameFromKey(int keycode) {
  return KeyCodeToName(keycode) + "_layer";
}

// Class methods.

ConfigParser::ConfigParser(Remapper* remapper) { remapper_ = remapper; }

[[nodiscard]] bool ConfigParser::Parse(const std::vector<string>& lines) {
  bool success = true;
  int line_num = 0;
  for (const auto& line : lines) {
    ++line_num;
    auto result = ParseLine(line);
    if (!result) {
      std::cerr << std::format("ERROR: {}\n  At line #{}, '{}'", result.error(),
                               line_num, line)
                << std::endl;
    }
    success &= result.has_value();
  }
  return success;
}

// PRIVATE

// Converts a string like "~D ^A" to actions.
std::expected<std::vector<Action>, std::string>
ConfigParser::AssignmentToActions(const string& assignment) {
  const auto tokens = StringSplit(assignment, ' ');
  return AssignmentToActions(tokens);
}

// Converts a vector<string> like ["B", "^C"] into vector<Action>.
std::expected<std::vector<Action>, std::string>
ConfigParser::AssignmentToActions(const std::vector<string>& tokens) {
  std::vector<Action> actions;

  for (const string& token : tokens) {
    if (token == kNothingToken || token == "^" + kNothingToken ||
        token == "~" + kNothingToken) {
      continue;
    }
    if (token.ends_with("ms")) {
      int ms;
      // This raises std::invalid_argument if number is invalid.
      try {
        ms = std::stoi(token.substr(0, token.size() - 2));
      } catch (std::invalid_argument&) {
        return std::unexpected(
            std::format("Could not parse waiting time {}.", token));
      }
      if (ms <= 0 || ms > kMaxWaitMs) {
        return std::unexpected(std::format("Out of range wait time {}ms", ms));
      }
      actions.push_back(ActionWait{ms});
      continue;
    }
    ASSIGN_OR_RETURN(const auto key, SplitKeyPrefix(token));
    if (!key.prefix.has_value() || key.prefix == '^') {
      actions.push_back(KeyPressEvent(key.key));
    }
    if (!key.prefix.has_value() || key.prefix == '~') {
      actions.push_back(KeyReleaseEvent(key.key));
    }
  }
  return actions;
}

// Given a key and string representing what it should do, adds relevant mappings
// to remapper_.
std::expected<void, std::string> ConfigParser::ParseAssignment(
    const string& layer_name, const string& key_str, const string& assignment) {
  ASSIGN_OR_RETURN(const auto left_key, SplitKeyPrefix(key_str));
  if (layer_name == kDefaultLayerName &&
      known_layers_.contains(LayerNameFromKey(left_key.key))) {
    return std::unexpected(
        "ERROR: Key assignments like KEY = ... must precede layer assignments "
        "KEY + OTHER_KEY = ...");
  }

  std::vector<string> tokens = StringSplit(assignment, ' ');
  if (tokens.size() == 1 && tokens[0] == "*") {
    tokens[0] = key_str;
  }
  // For assignments like A = B, convert to [^A = ^B, ~A = ~B].
  // And convert A = B C to [^A = B ^C, ~A = ~C].
  if (!left_key.prefix.has_value()) {
    int n_tokens = tokens.size();
    string last_token = tokens[n_tokens - 1];
    if (last_token[0] == '^' || last_token[0] == '~') {
      return std::unexpected(
          "ERROR: If left does not have a prefix (^ or ~), the last token of "
          "assignment must not have either.");
    }
    tokens[n_tokens - 1] = "^" + last_token;
    // On activation, do everything, but only activate the final key.
    {
      ASSIGN_OR_RETURN(const auto actions, AssignmentToActions(tokens));
      remapper_->AddMapping(layer_name, KeyPressEvent(left_key.key), actions);
    }
    // On release, do nothing, and only release the final key.
    {
      ASSIGN_OR_RETURN(const auto actions,
                       AssignmentToActions({"~" + last_token}));
      remapper_->AddMapping(layer_name, KeyReleaseEvent(left_key.key), actions);
    }
  } else {
    ASSIGN_OR_RETURN(const auto actions, AssignmentToActions(tokens));
    remapper_->AddMapping(layer_name,
                          left_key.prefix == '~' ? KeyReleaseEvent(left_key.key)
                                                 : KeyPressEvent(left_key.key),
                          actions);
  }
  return {};
}

std::expected<void, std::string> ConfigParser::ParseLayerAssignment(
    const string& layer_key_str, const string& key_str,
    const string& assignment) {
  // TODO: Clean up implicit throw. Use error message below.
  //  std::cerr << "ERROR: Could not parse layer key " << layer_key_str
  //            << std::endl;
  const auto layer_key = SplitKeyPrefix(layer_key_str).value();
  if (layer_key.prefix.has_value()) {
    return std::unexpected(
        "Prefix (^ or ~) for layer keys is not supported yet.");
  }
  string layer_name = LayerNameFromKey(layer_key.key);

  // Add default to layer mapping.
  if (known_layers_.find(layer_name) == known_layers_.end()) {
    remapper_->AddMapping(kDefaultLayerName, KeyPressEvent(layer_key.key),
                          {remapper_->ActionActivateState(layer_name)});
    remapper_->SetAllowOtherKeys(layer_name, false);
    known_layers_.insert(layer_name);
  }

  // Handle SHIFT + * = *.
  if (key_str == "*") {
    if (assignment != "*") {
      return std::unexpected(
          "Must be a * on the right side of for KEY + * = *");
    }
    remapper_->SetAllowOtherKeys(layer_name, true);
    return {};
  }

  // Handle DELETE + nothing = DELETE.
  if (key_str == kNothingToken) {
    ASSIGN_OR_RETURN(const auto actions, AssignmentToActions(assignment));
    remapper_->SetNullEventActions(layer_name, actions);
    return {};
  }

  return ParseAssignment(layer_name, key_str, assignment);
}

std::expected<void, std::string> ConfigParser::ParseLine(
    const string& original_line) {
  // Ignore comments and empty lines.
  string line = StringTrim(RemoveComment(original_line));
  if (line.empty()) {
    return {};
  }

  // Split the config line into the key combination and the action.
  auto parts = StringSplit(line, '=');
  if (parts.size() != 2) {
    return std::unexpected("ERROR: Not of the form A = B");
  }

  string key_combo = StringTrim(parts[0]);
  string action = StringTrim(parts[1]);

  // Split key combination by '+', e.g., "DEL + END"
  auto keys = StringSplit(key_combo, '+');

  if (keys.size() == 1) {
    return ParseAssignment(kDefaultLayerName, StringTrim(keys[0]), action);
  } else if (keys.size() == 2) {
    return ParseLayerAssignment(StringTrim(keys[0]), StringTrim(keys[1]),
                                action);
  } else {
    return std::unexpected(
        std::format("Cannot have more than 1 '+' in line: {}", original_line));
  }
  return {};
}
