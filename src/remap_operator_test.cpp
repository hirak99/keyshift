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

  CHECK(GetOutcomes(remapper, true,
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
  CHECK(GetOutcomes(remapper, false,
                    {{KEY_DELETE, 1},
                     {KEY_BACKSPACE, 1},
                     {KEY_DELETE, 0},
                     {KEY_BACKSPACE, 0}}) ==
        vector<string>{"Out: P KEY_PRINT", "Out: R KEY_PRINT"});
  // Leave the other key first.
  CHECK(GetOutcomes(remapper, false,
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
  CHECK(GetOutcomes(remapper, false,
                    {{KEY_RIGHTCTRL, 1},
                     {KEY_BACKSPACE, 1},
                     {KEY_RIGHTCTRL, 0},
                     {KEY_BACKSPACE, 0},
                     {KEY_BACKSPACE, 1}}) ==
        vector<string>{"Out: P KEY_RIGHTCTRL", "Out: P KEY_A", "Out: R KEY_A",
                       "Out: R KEY_RIGHTCTRL", "Out: P KEY_BACKSPACE"});
  // Covers all other keys as expected.
  CHECK(GetOutcomes(
            remapper, false,
            {{KEY_RIGHTCTRL, 1}, {KEY_B, 1}, {KEY_RIGHTCTRL, 0}, {KEY_B, 0}}) ==
        vector<string>{"Out: P KEY_RIGHTCTRL", "Out: P KEY_B",
                       "Out: R KEY_RIGHTCTRL", "Out: R KEY_B"});
  // However, as defined releases CTRL before F1.
  CHECK(GetOutcomes(
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

    THEN("Del+End does VolumeUp") {
      CHECK(
          GetOutcomes(
              remapper, false,
              {{KEY_DELETE, 1}, {KEY_END, 1}, {KEY_END, 0}, {KEY_DELETE, 0}}) ==
          vector<string>{"Out: P KEY_VOLUMEUP", "Out: R KEY_VOLUMEUP"});
    }

    THEN("Del alone should act as Del key") {
      CHECK(GetOutcomes(remapper, false, {{KEY_DELETE, 1}, {KEY_DELETE, 0}}) ==
            vector<string>{"Out: P KEY_DELETE", "Out: R KEY_DELETE"});
    }
  }
}

SCENARIO("Must repeat - both mapped and passthru keys") {
  GIVEN("Set A=B") {
    Remapper remapper;

    remapper.AddMapping("", KeyPressEvent(KEY_A), {KeyPressEvent(KEY_B)});
    remapper.AddMapping("", KeyReleaseEvent(KEY_A), {KeyReleaseEvent(KEY_B)});

    THEN("Repeat A repeats B") {
      CHECK(GetOutcomes(remapper, false,
                        {{KEY_A, 1}, {KEY_A, 2}, {KEY_A, 2}, {KEY_A, 0}}) ==
            vector<string>{"Out: P KEY_B", "Out: T KEY_B", "Out: T KEY_B",
                           "Out: R KEY_B"});
    }

    THEN("Repeat C repeats C") {
      CHECK(GetOutcomes(remapper, false,
                        {{KEY_C, 1}, {KEY_C, 2}, {KEY_C, 2}, {KEY_C, 0}}) ==
            vector<string>{"Out: P KEY_C", "Out: T KEY_C", "Out: T KEY_C",
                           "Out: R KEY_C"});
    }
  }
}
