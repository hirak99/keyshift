#include "config_parser.h"

#include <catch2/catch_test_macros.hpp>

#include "remap_operator.h"
#include "test_utils.h"

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

// Swap 1 and 2.
1 = 2
2 = 1
)";

const std::string kExpectedDump = R"(State #0
  Other keys: Allow
  On: (KEY_2 Release)
    Key: (KEY_1 Release)
  On: (KEY_1 Press)
    Key: (KEY_2 Press)
  On: (KEY_1 Release)
    Key: (KEY_2 Release)
  On: (KEY_A Press)
    Key: (KEY_D Release)
    Key: (KEY_A Press)
  On: (KEY_DELETE Press)
    Layer Change: 4
  On: (KEY_2 Press)
    Key: (KEY_1 Press)
  On: (KEY_LEFTSHIFT Press)
    Key: (KEY_LEFTSHIFT Press)
    Layer Change: 3
  On: (KEY_RIGHTCTRL Press)
    Key: (KEY_RIGHTCTRL Press)
    Layer Change: 2
  On: (KEY_CAPSLOCK Press)
    Layer Change: 1
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
State #2
  Other keys: Allow
  On: (KEY_1 Release)
    Key: (KEY_F1 Release)
  On: (KEY_1 Press)
    Key: (KEY_RIGHTCTRL Release)
    Key: (KEY_F1 Press)
State #3
  Other keys: Block
  On: (KEY_ESC Release)
    Key: (KEY_GRAVE Release)
  On: (KEY_ESC Press)
    Key: (KEY_GRAVE Press)
State #4
  Other keys: Block
  On: (KEY_END Release)
    Key: (KEY_VOLUMEUP Release)
  On: (KEY_END Press)
    Key: (KEY_VOLUMEUP Press)
  On nothing:
    Key: (KEY_DELETE Press)
    Key: (KEY_DELETE Release)
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

SCENARIO("All mappings") {
  GIVEN("All mappings") {
    Remapper remapper;
    ConfigParser config_parser(&remapper);

    auto config_lines = SplitLines(kConfigLines);
    REQUIRE(config_parser.Parse(config_lines));

    THEN("Dump is as expected") {
      REQUIRE(GetRemapperConfigDump(remapper) == kExpectedDump);
    }

    // Functional tests.
    THEN("Test outcome CAPSLOCK+Fn") {
      REQUIRE(
          GetOutcomes(
              remapper, false,
              {{KEY_CAPSLOCK, 1}, {KEY_1, 1}, {KEY_1, 0}, {KEY_CAPSLOCK, 0}}) ==
          vector<string>{"Out: P KEY_F1", "Out: R KEY_F1"});
      REQUIRE(
          GetOutcomes(
              remapper, false,
              {{KEY_CAPSLOCK, 1}, {KEY_1, 1}, {KEY_CAPSLOCK, 0}, {KEY_1, 0}}) ==
          vector<string>{"Out: P KEY_F1", "Out: R KEY_F1"});
    }
    THEN("Test repeat of keys") {
      REQUIRE(GetOutcomes(remapper, false,
                          {{KEY_X, 1}, {KEY_X, 2}, {KEY_X, 2}, {KEY_X, 0}}) ==
              vector<string>{"Out: P KEY_X", "Out: T KEY_X", "Out: T KEY_X",
                             "Out: R KEY_X"});
    }
    THEN("Test repeat of CAPSLOCK (lead) and 1 (mapped to F1)") {
      REQUIRE(GetOutcomes(remapper, false,
                          {{KEY_CAPSLOCK, 1},
                           {KEY_CAPSLOCK, 2},
                           {KEY_1, 1},
                           {KEY_1, 2},
                           {KEY_CAPSLOCK, 0},
                           {KEY_1, 0}}) ==
              vector<string>{"Out: P KEY_F1",
                             // Note: In current config, the mapped key does not
                             // repeat. However, repeating this also should be
                             // okay. "Out: T KEY_F1",
                             "Out: R KEY_F1"});
    }
  }
}

SCENARIO("Custom tests") {
  Remapper remapper;
  ConfigParser config_parser(&remapper);

  GIVEN("KEY + X = X") {
    REQUIRE(config_parser.Parse({"CAPSLOCK + LEFTALT = LEFTALT"}));

    THEN("Dump is as expected") {
      REQUIRE(GetRemapperConfigDump(remapper) == R"(State #0
  Other keys: Allow
  On: (KEY_CAPSLOCK Press)
    Layer Change: 1
State #1
  Other keys: Block
  On: (KEY_LEFTALT Release)
    Key: (KEY_LEFTALT Release)
  On: (KEY_LEFTALT Press)
    Key: (KEY_LEFTALT Press)
)");
    }
  }

  GIVEN("ALT + CAPS + 4 = ALT F4") {
    REQUIRE(config_parser.Parse(
        {"CAPSLOCK + LEFTALT = LEFTALT", "CAPSLOCK + 4 = F4"}));
    THEN("CAPS + ALT + 4") {
      REQUIRE(GetOutcomes(remapper, false,
                          {{KEY_CAPSLOCK, 1},
                           {KEY_LEFTALT, 1},
                           {KEY_4, 1},
                           {KEY_LEFTALT, 0},
                           {KEY_CAPSLOCK, 0},
                           {KEY_4, 0}}) ==
              vector<string>{"Out: P KEY_LEFTALT", "Out: P KEY_F4",
                             "Out: R KEY_LEFTALT", "Out: R KEY_F4"});
    }
    THEN("ALT + CAPS + 4") {
      REQUIRE(GetOutcomes(remapper, false,
                          {{KEY_LEFTALT, 1},
                           {KEY_CAPSLOCK, 1},
                           {KEY_4, 1},
                           {KEY_LEFTALT, 0},
                           {KEY_CAPSLOCK, 0},
                           {KEY_4, 0}}) ==
              vector<string>{"Out: P KEY_LEFTALT", "Out: P KEY_F4",
                             "Out: R KEY_LEFTALT", "Out: R KEY_F4"});
    }
  }

  // Handle multiple layers being active at the same time.
  //   E.g. Alt + Caps + 4 = Alt F4
  //   The modifications should compose in order of layers activated.
  GIVEN("Multiple layers") {
    REQUIRE(config_parser.Parse({"CAPSLOCK + LEFTALT = LEFTALT",
                                 "CAPSLOCK + 4 = F4", "^LEFTALT = ^LEFTALT",
                                 "LEFTALT + * = *"}));
    THEN("CAPS + ALT + 4") {
      REQUIRE(GetOutcomes(remapper, false,
                          {{KEY_CAPSLOCK, 1},
                           {KEY_LEFTALT, 1},
                           {KEY_4, 1},
                           {KEY_LEFTALT, 0},
                           {KEY_CAPSLOCK, 0},
                           {KEY_4, 0}}) ==
              vector<string>{"Out: P KEY_LEFTALT", "Out: P KEY_F4",
                             "Out: R KEY_LEFTALT", "Out: R KEY_F4"});
    }
    THEN("ALT + CAPS + 4") {
      REQUIRE(GetOutcomes(remapper, false,
                          {{KEY_LEFTALT, 1},
                           {KEY_CAPSLOCK, 1},
                           {KEY_4, 1},
                           {KEY_LEFTALT, 0},
                           {KEY_CAPSLOCK, 0},
                           {KEY_4, 0}}) ==
              vector<string>{"Out: P KEY_LEFTALT", "Out: P KEY_F4",
                             "Out: R KEY_LEFTALT", "Out: R KEY_F4"});
    }
  }
}
