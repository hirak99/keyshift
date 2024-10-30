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

#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "keycode_lookup.h"
#include "remap_operator.h"

using std::string;

class ConfigParser {
 public:
  ConfigParser(Remapper* remapper);
  [[nodiscard]] bool Parse(const std::vector<string>& lines);

  // Movable but not copyable.
  ConfigParser(ConfigParser&& other) = default;
  ConfigParser& operator=(ConfigParser&& other) = default;

 private:
  // Converts a string like "~D ^A" to actions.
  std::vector<Action> AssignmentToActions(const string& assignment);

  std::vector<Action> AssignmentToActions(const std::vector<string>& tokens);

  bool ParseAssignment(const string& layer_name, const string& key_str,
                       const string& assignment);

  bool ParseLayerAssignment(const string& layer_key_str, const string& key_str,
                            const string& assignment);

  [[nodiscard]] bool ParseLine(const string& original_line);

  Remapper* remapper_;
  // To keep track of which layers have been seen. Used to do one time actions,
  // such as disallow other keys.
  std::set<string> known_layers_;
};
