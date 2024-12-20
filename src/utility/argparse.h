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

// Simple argument parser.
// Boost program_options does not appear to do a lot more, but requires linking
// to a library. Might as well roll our own for simple tasks.
//
// Member functions should be self evident.
// Usage example -
//
// int main(const int argc, const char** argv) {
//   ArgumentParser parser;
//   parser.AddBool("help", "Show a short help.");
//   parser.AddString("filename", "Set filename");
//   parser.Parse(argc, argv);
//   if (arg_help) {
//     parser.ShowHelp();
//     return;
//   }
//   std::cout << parser.GetString("filename").value();
// }
//
#ifndef __ARGPARSE_H
#define __ARGPARSE_H

#include <expected>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "essentials.h"

class ArgumentParser {
 public:
  void AddBool(const std::string& name, const std::string& desc);
  void AddString(const std::string& name, const std::string& desc);

  void ShowHelp();

  [[nodiscard]]
  std::expected<void, std::string> Parse(const int argc, const char** argv);

  bool GetBool(const std::string& name);
  std::optional<std::string> GetString(const std::string& name);
  std::string GetRequiredString(const std::string& name);

 private:
  enum ArgType { UNKNOWN, BOOLEAN, STRING };

  void AddHelp(const std::string& option, const std::string& description);
  void ConfirmType(const std::string& name, const ArgType arg_type);

  std::unordered_map<std::string, ArgType> argument_types_;
  std::unordered_map<std::string, std::string> argument_values_;
  std::vector<std::pair<std::string, std::string>> help_lines_;
};

#endif  // __ARGPARSE_H