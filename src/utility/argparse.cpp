#include "argparse.h"

#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "essentials.h"

using std::string;

const std::size_t kMaxCol1Width = 32;

void ArgumentParser::AddBool(const string& name, const string& desc) {
  argument_types_[name] = ArgType::BOOLEAN;
  AddHelp("--" + name, desc);
}
void ArgumentParser::AddString(const string& name, const string& desc) {
  argument_types_[name] = ArgType::STRING;
  AddHelp("--" + name + "=STRING", desc);
}

void ArgumentParser::ShowHelp() {
  std::cout << "Allowed options:" << std::endl;
  std::size_t max_option_len = 0;
  for (const auto& [option, desc] : help_lines_) {
    if (option.size() > max_option_len) {
      max_option_len = option.size();
    }
  }
  int col1_width = std::min(max_option_len + 2, kMaxCol1Width - 1);
  for (const auto& [option, desc] : help_lines_) {
    int gap = col1_width - 1 - option.size();
    if (gap < 0) gap = 0;
    const string spaces(gap, ' ');
    std::cout << option << spaces << " " << desc << std::endl;
  }
}

void ArgumentParser::Parse(const int argc, const char** argv) {
  // If clear, next token will be argument.
  // If populated, next token is the value.
  std::optional<string> this_arg;
  ArgType this_arg_type = ArgType::UNKNOWN;
  for (int i = 1; i < argc; ++i) {
    string token = argv[i];
    if (!this_arg) {
      if (token.size() >= 2 && token.substr(0, 2) == "--") {
        token = token.substr(2);
      } else {
        throw std::invalid_argument("Cannot parse argument " + token);
      }

      std::optional<string> value;
      std::size_t eq_pos = token.find('=');
      if (eq_pos != std::string::npos) {
        value = token.substr(eq_pos + 1);
        token = token.substr(0, eq_pos);
      }

      // Next arg should be argument name.
      auto it = argument_types_.find(token);
      if (it == argument_types_.end()) {
        throw std::invalid_argument(std::format("Unknown argument {}", token));
      }
      if (it->second == ArgType::BOOLEAN) {
        if (value) {
          value = ToLowerCase(value.value());
          if (value == "true" || value == "yes") {
            argument_values_[token] = "";
          }
        } else {
          argument_values_[token] = "";
        }
      } else {
        if (value) {
          argument_values_[token] = value.value();
        } else {
          this_arg.emplace(token);
          this_arg_type = it->second;
        }
      }
    } else {
      // Next arg should be argument value.
      if (token.size() >= 1 && token[0] == '-') {
        throw std::invalid_argument("Expecting value for " + this_arg.value() +
                                    " but found " + token);
      }
      switch (this_arg_type) {
        case ArgType::STRING:
          argument_values_[*this_arg] = token;
          break;
        default:
          throw std::runtime_error("Unexpected argument type");
      }
      this_arg = std::nullopt;
    }
  }
  if (this_arg.has_value()) {
    throw std::invalid_argument("Value of argument not found: " +
                                this_arg.value());
  }
}

bool ArgumentParser::GetBool(const string& name) {
  ConfirmType(name, ArgType::BOOLEAN);
  return argument_values_.contains(name);
}

std::optional<string> ArgumentParser::GetString(const string& name) {
  ConfirmType(name, ArgType::STRING);
  return MapLookup(argument_values_, name);
}

string ArgumentParser::GetRequiredString(const string& name) {
  auto result = MapLookup(argument_values_, name);
  if (!result) {
    throw std::invalid_argument("Required argument " + name + " is missing.");
  }
  return *result;
}

// Private.

void ArgumentParser::AddHelp(const string& option, const string& description) {
  help_lines_.push_back({" " + option, description});
}

void ArgumentParser::ConfirmType(const string& name, const ArgType arg_type) {
  auto it = argument_types_.find(name);
  if (it == argument_types_.end()) {
    throw std::runtime_error("Unknown argument in Get... for argument " + name);
  }
  if (it->second != arg_type) {
    throw std::runtime_error("Invalid argument in Get... for argument " + name);
  }
}
