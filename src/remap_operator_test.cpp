#include "remap_operator.h"

#include <linux/input-event-codes.h>
#include <stdlib.h>

#include <catch2/catch_test_macros.hpp>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "keycode_lookup.h"
#include "test_utils.h"

using std::string;
using std::vector;

TEST_CASE("Test1", "[remapper]") {
  Remapper remapper;

  remapper.AddMapping("fnkeys", KeyPressEvent(KEY_A), {KeyPressEvent(KEY_B)});
  remapper.AddMapping("fnkeys", KeyReleaseEvent(KEY_A),
                      {KeyReleaseEvent(KEY_B)});
  remapper.AddMapping("fnkeys", KeyPressEvent(KEY_1), {KeyPressEvent(KEY_F1)});
  remapper.AddMapping("fnkeys", KeyPressEvent(KEY_0), {KeyPressEvent(KEY_F10)});
  remapper.AddMapping(
      "", KeyPressEvent(KEY_RIGHTCTRL),
      {KeyPressEvent(KEY_RIGHTCTRL), remapper.ActionActivateState("fnkeys")});

  REQUIRE(GetOutcomes(remapper, true,
                      {{KEY_C, 1},
                       {KEY_C, 0},
                       {KEY_RIGHTCTRL, 1},
                       {KEY_A, 1},
                       {KEY_RIGHTCTRL, 0},
                       {KEY_A, 1},
                       {KEY_A, 0}}) ==

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

  remapper.AddMapping("", KeyPressEvent(KEY_DELETE),
                      {remapper.ActionActivateState("del")});
  remapper.AddMapping("del", KeyPressEvent(KEY_BACKSPACE),
                      {KeyPressEvent(KEY_PRINT)});

  // Leave the lead key first.
  REQUIRE(GetOutcomes(remapper, false,
                      {{KEY_DELETE, 1},
                       {KEY_BACKSPACE, 1},
                       {KEY_DELETE, 0},
                       {KEY_BACKSPACE, 0}}) ==
          vector<string>{"Out: P KEY_PRINT", "Out: R KEY_PRINT"});
  // Leave the other key first.
  REQUIRE(GetOutcomes(remapper, false,
                      {{KEY_DELETE, 1},
                       {KEY_BACKSPACE, 1},
                       {KEY_BACKSPACE, 0},
                       {KEY_DELETE, 0}}) ==
          vector<string>{"Out: P KEY_PRINT", "Out: R KEY_PRINT"});
}

TEST_CASE("RCtrl deactivates around F-keys", "[remapper]") {
  Remapper remapper;

  remapper.AddMapping("", KeyPressEvent(KEY_RIGHTCTRL),
                      {KeyPressEvent(KEY_RIGHTCTRL),
                       remapper.ActionActivateState("rctrl_fn_layer")});
  remapper.AddMapping("rctrl_fn_layer", KeyPressEvent(KEY_BACKSPACE),
                      {KeyPressEvent(KEY_A)});
  // Ctrl will be released if 0 is pressed.
  remapper.AddMapping("rctrl_fn_layer", KeyPressEvent(KEY_1),
                      {KeyReleaseEvent(KEY_RIGHTCTRL), KeyPressEvent(KEY_F1)});

  // Covers mapped keys.
  REQUIRE(GetOutcomes(remapper, false,
                      {{KEY_RIGHTCTRL, 1},
                       {KEY_BACKSPACE, 1},
                       {KEY_RIGHTCTRL, 0},
                       {KEY_BACKSPACE, 0},
                       {KEY_BACKSPACE, 1}}) ==
          vector<string>{"Out: P KEY_RIGHTCTRL", "Out: P KEY_A", "Out: R KEY_A",
                         "Out: R KEY_RIGHTCTRL", "Out: P KEY_BACKSPACE"});
  // Covers all other keys as expected.
  REQUIRE(
      GetOutcomes(
          remapper, false,
          {{KEY_RIGHTCTRL, 1}, {KEY_B, 1}, {KEY_RIGHTCTRL, 0}, {KEY_B, 0}}) ==
      vector<string>{"Out: P KEY_RIGHTCTRL", "Out: P KEY_B", "Out: R KEY_B",
                     "Out: R KEY_RIGHTCTRL"});
  // However, as defined releases CTRL before F1.
  REQUIRE(
      GetOutcomes(
          remapper, false,
          {{KEY_RIGHTCTRL, 1}, {KEY_1, 1}, {KEY_1, 0}, {KEY_RIGHTCTRL, 0}}) ==
      vector<string>{"Out: P KEY_RIGHTCTRL", "Out: R KEY_RIGHTCTRL",
                     "Out: P KEY_F1", "Out: R KEY_F1"});
}

SCENARIO("Del+Bksp is Print, but Del alone is Del") {
  GIVEN("Remapper with del") {
    Remapper remapper;

    remapper.AddMapping("", KeyPressEvent(KEY_DELETE),
                        {remapper.ActionActivateState("del_layer")});
    remapper.SetAllowOtherKeys("del_layer", false);
    remapper.AddMapping("del_layer", KeyPressEvent(KEY_END),
                        {KeyPressEvent(KEY_VOLUMEUP)});
    remapper.AddMapping("del_layer", KeyReleaseEvent(KEY_END),
                        {KeyReleaseEvent(KEY_VOLUMEUP)});
    remapper.SetNullEventActions(
        "del_layer", {KeyPressEvent(KEY_DELETE), KeyReleaseEvent(KEY_DELETE)});

    // WIP - Does not pass yet.
    THEN("Del+End does VolumeUp") {
      REQUIRE(
          GetOutcomes(
              remapper, false,
              {{KEY_DELETE, 1}, {KEY_END, 1}, {KEY_END, 0}, {KEY_DELETE, 0}}) ==
          vector<string>{"Out: P KEY_VOLUMEUP", "Out: R KEY_VOLUMEUP"});
    }

    THEN("Del alone should act as Del key") {
      REQUIRE(
          GetOutcomes(remapper, false, {{KEY_DELETE, 1}, {KEY_DELETE, 0}}) ==
          vector<string>{"Out: P KEY_DELETE", "Out: R KEY_DELETE"});
    }
  }
}