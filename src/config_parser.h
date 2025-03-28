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
#ifndef __CONFIG_PARSER_H
#define __CONFIG_PARSER_H

#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "keycode_lookup.h"
#include "remap_operator.h"
#include "utility/essentials.h"

class ConfigParser {
 public:
  ConfigParser(Remapper* remapper);
  [[nodiscard]] bool Parse(const std::vector<std::string>& lines);

  // Movable but not copyable.
  ConfigParser(ConfigParser&& other) = default;
  ConfigParser& operator=(ConfigParser&& other) = default;

 private:
  // Converts a std::string like "~D ^A" to actions.
  ErrorStrOr<std::vector<Action>> AssignmentToActions(
      const std::string& assignment);

  ErrorStrOr<std::vector<Action>> AssignmentToActions(
      const std::vector<std::string>& tokens);

  ErrorStrOr<void> ParseAssignment(const std::string& layer_name,
                                   const std::string& key_str,
                                   const std::string& assignment);

  ErrorStrOr<void> ParseLayerAssignment(const std::string& layer_key_str,
                                        const std::string& key_str,
                                        const std::string& assignment);

  [[nodiscard]] ErrorStrOr<void> ParseLine(const std::string& original_line);

  Remapper* remapper_;
  // To keep track of keys used to define layers.
  std::set<int> layer_keys_;
};

#endif  //  __CONFIG_PARSER_H