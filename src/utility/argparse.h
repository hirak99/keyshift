// Simple argument parser.
// Boost program_options does not appear to do a lot more, but requires linking
// to a library. Might as well roll our own for simple tasks.
#ifndef __ARGPARSE_H
#define __ARGPARSE_H

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

  void Parse(const int argc, const char** argv);

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