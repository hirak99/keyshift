#include "argparse.h"

#include <catch2/catch_test_macros.hpp>

// Helper to call parse, with the correct argc count.
void CallParse(ArgumentParser& parser, const std::vector<std::string> args) {
  // Create an array of const char* and initialize it from the std::vector
  std::vector<const char*> argv(args.size() + 1);  // +1 for NULL terminator

  for (std::size_t i = 0; i < args.size(); ++i) {
    argv[i] = args[i].c_str();  // Use .c_str() to get const char*
  }

  argv[args.size()] = nullptr;  // Set last element to NULL

  // Simulating argc.
  int argc = static_cast<int>(args.size());

  parser.Parse(argc, argv.data());
}

SCENARIO("Exception on unknown arg") {
  ArgumentParser parser;
  parser.AddBool("help", "Show help.");
  CHECK_THROWS_AS(CallParse(parser, {"COMMAND", "--argname1", "argvalue1"}),
                  std::invalid_argument);
}

SCENARIO("Incomplete argument") {
  ArgumentParser parser;
  parser.AddString("name", "This argument must have a value.");

  THEN("End without specifying value") {
    CHECK_THROWS_AS(CallParse(parser, {"COMMAND", "--name"}),
                    std::invalid_argument);
  }

  THEN("Another argument without specifying value") {
    CHECK_THROWS_AS(CallParse(parser, {"COMMAND", "--name", "--help"}),
                    std::invalid_argument);
  }
}

SCENARIO("Parsing space delimited args") {
  ArgumentParser parser;
  parser.AddBool("help", "Show help.");
  parser.ShowHelp();

  CallParse(parser, {"COMMAND", "--help"});
}