#include "remap_operator.h"

#include <linux/input-event-codes.h>
#include <stdlib.h>

#include <catch2/catch_test_macros.hpp>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "keycode_lookup.h"

using std::string;
using std::vector;

// Helper functions.
string Join(const vector<string>& vec) {
  const string delimiter = ", ";
  std::ostringstream oss;
  oss << "[";
  for (std::size_t i = 0; i < vec.size(); ++i) {
    if (i != 0) {
      oss << delimiter;
    }
    oss << "\"" << vec[i] << "\"";
  }
  oss << "]";
  return oss.str();
}

vector<string> GetOutcomes(Remapper& remapper, bool keep_incoming,
                           vector<int> keycodes) {
  vector<string> outcomes;
  auto process = [&outcomes, &remapper, keep_incoming](int keycode) {
    if (keep_incoming) {
      std::ostringstream oss;
      oss << "In: " << (keycode > 0 ? "P " : "R ")
          << keyCodeToString(abs(keycode));
      outcomes.push_back(oss.str());
    }
    remapper.process(keycode);
  };

  remapper.SetCallback([&outcomes](int keycode, int press) {
    std::ostringstream oss;
    oss << "Out: " << (press == 1 ? "P " : "R ") << keyCodeToString(keycode);
    outcomes.push_back(oss.str());
  });

  for (int keycode : keycodes) {
    process(keycode);
  }

  return outcomes;
}

TEST_CASE("Test1", "[remapper]") {
  Remapper remapper;

  remapper.add_mapping("fnkeys", KEY_A, {remapper.action_key(KEY_B)});
  remapper.add_mapping("fnkeys", -KEY_A, {remapper.action_key(-KEY_B)});
  remapper.add_mapping("fnkeys", KEY_1, {remapper.action_key(KEY_F1)});
  remapper.add_mapping("fnkeys", KEY_0, {remapper.action_key(KEY_F10)});
  remapper.add_mapping("", KEY_RIGHTCTRL,
                       {remapper.action_key(KEY_RIGHTCTRL),
                        remapper.action_activate_mapping("fnkeys")});

  REQUIRE(GetOutcomes(remapper, true,
                      {KEY_C, -KEY_C, KEY_RIGHTCTRL, KEY_A, -KEY_RIGHTCTRL,
                       KEY_A, -KEY_A}) ==

          vector<string>{
              "In: P KEY_C",
              "Out: P KEY_C",
              "In: R KEY_C",
              "Out: R KEY_C",
              "In: P KEY_RIGHTCTRL",
              "Out: P KEY_RIGHTCTRL",
              "In: P KEY_A",
              "Out: P KEY_B",
              "In: R KEY_RIGHTCTRL",
              "Out: R KEY_B",
              "Out: R KEY_RIGHTCTRL",
              "In: P KEY_A",
              "Out: P KEY_A",
              "In: R KEY_A",
              "Out: R KEY_A",
          });
}

TEST_CASE("Lead key", "[remapper]") {
  Remapper remapper;

  remapper.add_mapping("del", KEY_BACKSPACE, {remapper.action_key(KEY_PRINT)});
  remapper.add_mapping("", KEY_DELETE,
                       {remapper.action_activate_mapping("del")});

  // Leave the lead key first.
  REQUIRE(
      GetOutcomes(remapper, false,
                  {KEY_DELETE, KEY_BACKSPACE, -KEY_DELETE, -KEY_BACKSPACE}) ==
      vector<string>{"Out: P KEY_PRINT", "Out: R KEY_PRINT"});
  // Leave the other key first.
  REQUIRE(
      GetOutcomes(remapper, false,
                  {KEY_DELETE, KEY_BACKSPACE, -KEY_BACKSPACE, -KEY_DELETE}) ==
      vector<string>{"Out: P KEY_PRINT", "Out: R KEY_PRINT"});
}
