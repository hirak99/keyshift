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
    remapper.process(abs(keycode), keycode > 0 ? 1 : 0);
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

  remapper.add_mapping("fnkeys", KeyPressEvent(KEY_A), {KeyPressEvent(KEY_B)});
  remapper.add_mapping("fnkeys", KeyReleaseEvent(KEY_A),
                       {KeyReleaseEvent(KEY_B)});
  remapper.add_mapping("fnkeys", KeyPressEvent(KEY_1), {KeyPressEvent(KEY_F1)});
  remapper.add_mapping("fnkeys", KeyPressEvent(KEY_0),
                       {KeyPressEvent(KEY_F10)});
  remapper.add_mapping("", KeyPressEvent(KEY_RIGHTCTRL),
                       {KeyPressEvent(KEY_RIGHTCTRL),
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

  remapper.add_mapping("", KeyPressEvent(KEY_DELETE),
                       {remapper.action_activate_mapping("del")});
  remapper.add_mapping("del", KeyPressEvent(KEY_BACKSPACE),
                       {KeyPressEvent(KEY_PRINT)});

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

TEST_CASE("RCtrl deactivates around F-keys", "[remapper]") {
  Remapper remapper;

  remapper.add_mapping("", KeyPressEvent(KEY_RIGHTCTRL),
                       {KeyPressEvent(KEY_RIGHTCTRL),
                        remapper.action_activate_mapping("rctrl_fn_layer")});
  remapper.add_mapping("rctrl_fn_layer", KeyPressEvent(KEY_BACKSPACE),
                       {KeyPressEvent(KEY_A)});
  // Ctrl will be released if 0 is pressed.
  remapper.add_mapping("rctrl_fn_layer", KeyPressEvent(KEY_1),
                       {KeyReleaseEvent(KEY_RIGHTCTRL), KeyPressEvent(KEY_F1)});

  // Covers mapped keys.
  REQUIRE(GetOutcomes(remapper, false,
                      {KEY_RIGHTCTRL, KEY_BACKSPACE, -KEY_RIGHTCTRL,
                       -KEY_BACKSPACE, KEY_BACKSPACE}) ==
          vector<string>{"Out: P KEY_RIGHTCTRL", "Out: P KEY_A", "Out: R KEY_A",
                         "Out: R KEY_RIGHTCTRL", "Out: P KEY_BACKSPACE"});
  // Covers all other keys as expected.
  REQUIRE(GetOutcomes(remapper, false,
                      {KEY_RIGHTCTRL, KEY_B, -KEY_RIGHTCTRL, -KEY_B}) ==
          vector<string>{"Out: P KEY_RIGHTCTRL", "Out: P KEY_B", "Out: R KEY_B",
                         "Out: R KEY_RIGHTCTRL"});
  // However, as defined releases CTRL before F1.
  REQUIRE(GetOutcomes(remapper, false,
                      {KEY_RIGHTCTRL, KEY_1, -KEY_1, -KEY_RIGHTCTRL}) ==
          vector<string>{"Out: P KEY_RIGHTCTRL", "Out: R KEY_RIGHTCTRL",
                         "Out: P KEY_F1", "Out: R KEY_F1"});
}

SCENARIO("Del+Bksp is Print, but Del alone is Del") {
  GIVEN("Remapper with del") {
    Remapper remapper;

    remapper.add_mapping("", KeyPressEvent(KEY_DELETE),
                         {remapper.action_activate_mapping("del_layer")});
    remapper.add_mapping(
        "del_layer", KeyReleaseEvent(KEY_DELETE),
        {KeyPressEvent(KEY_DELETE), KeyReleaseEvent(KEY_DELETE),
         ActionDeactivateLayer{}});
    remapper.add_mapping(
        "del_layer", KeyReleaseEvent(KEY_END),
        {KeyPressEvent(KEY_VOLUMEUP), KeyReleaseEvent(KEY_VOLUMEUP),
         ActionDeactivateLayer{}});

    // WIP - Does not pass yet.
    THEN("Del+End does VolumeUp") {
      REQUIRE(GetOutcomes(remapper, false,
                          {KEY_DELETE, KEY_END, -KEY_END, -KEY_DELETE}) ==
              vector<string>{"Out: P KEY_VOLUMEUP", "Out: R KEY_VOLUMEUP"});
    }

    THEN("Del alone should act as Del key") {
      REQUIRE(GetOutcomes(remapper, false, {KEY_DELETE, -KEY_DELETE}) ==
              vector<string>{"Out: P KEY_DELETE", "Out: R KEY_DELETE"});
    }
  }
}