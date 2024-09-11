#include "config_parser.h"

#include <catch2/catch_test_macros.hpp>

#include "remap_operator.h"

std::string GetRemapperConfigDump(const Remapper& remapper) {
  std::ostringstream oss;
  remapper.DumpConfig(oss);
  return oss.str();
}

const std::string kConfigLines = R"(
CAPSLOCK + 1 = F1
CAPSLOCK + 2 = F2

^RIGHTCTRL = ^RIGHTCTRL
RIGHTCTRL + 1 = ~RIGHTCTRL F1
RIGHTCTRL + * = *

^LEFTSHIFT = ^LEFTSHIFT
LEFTSHIFT + ESC = GRAVE

DELETE + END = VOLUMEUP
DELETE + nothing = DELETE

// Snap tap.
^A = ~D ^A
)";

const std::string kExpectedDump = R"(State #4
  Other keys: Block
  On: (KEY_END Release)
    Key: (KEY_VOLUMEUP Release)
  On: (KEY_END Press)
    Key: (KEY_VOLUMEUP Press)
  On nothing:
    Key: (KEY_DELETE Press)
    Key: (KEY_DELETE Release)
State #3
  Other keys: Block
  On: (KEY_ESC Release)
    Key: (KEY_GRAVE Release)
  On: (KEY_ESC Press)
    Key: (KEY_GRAVE Press)
State #2
  Other keys: Allow
  On: (KEY_1 Release)
    Key: (KEY_F1 Release)
  On: (KEY_1 Press)
    Key: (KEY_RIGHTCTRL Release)
    Key: (KEY_F1 Press)
State #1
  Other keys: Block
  On: (KEY_2 Release)
    Key: (KEY_F2 Release)
  On: (KEY_2 Press)
    Key: (KEY_F2 Press)
  On: (KEY_1 Release)
    Key: (KEY_F1 Release)
  On: (KEY_1 Press)
    Key: (KEY_F1 Press)
State #0
  Other keys: Allow
  On: (KEY_A Press)
    Key: (KEY_D Release)
    Key: (KEY_A Press)
  On: (KEY_DELETE Press)
    Layer Change: 4
  On: (KEY_LEFTSHIFT Press)
    Key: (KEY_LEFTSHIFT Press)
    Layer Change: 3
  On: (KEY_RIGHTCTRL Press)
    Key: (KEY_RIGHTCTRL Press)
    Layer Change: 2
  On: (KEY_CAPSLOCK Press)
    Layer Change: 1
)";

std::vector<string> SplitLines(const string& str) {
  std::vector<string> lines;
  string line;
  std::istringstream line_stream(str);
  while (std::getline(line_stream, line, '\n')) {
    lines.push_back(line);
  }
  return lines;
}

SCENARIO("All mappings together") {
  GIVEN("All mappings together") {
    Remapper remapper;
    ConfigParser config_parser(&remapper);

    auto config_lines = SplitLines(kConfigLines);

    // Parse the config and set up the remapper.
    if (!config_parser.Parse(config_lines)) {
      perror("Could not parse config, exiting.\n");
      FAIL();
    }

    THEN("Dump is as expected") {
      REQUIRE(GetRemapperConfigDump(remapper) == kExpectedDump);
    }
  }
}
