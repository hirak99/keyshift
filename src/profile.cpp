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

#include "config_parser.h"
#include "remap_operator.h"

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

std::vector<std::string> SplitLines(const std::string& str) {
  std::vector<std::string> lines;
  std::string line;
  std::istringstream line_stream(str);
  while (std::getline(line_stream, line, '\n')) {
    lines.push_back(line);
  }
  return lines;
}

int main() {
  Remapper remapper;
  ConfigParser config_parser(&remapper);
  auto config_lines = SplitLines(kConfigLines);
  if (!config_parser.Parse(config_lines)) {
    throw std::runtime_error("Could not parse the config!");
  }

  for (int i = 0; i < 2000000; ++i) {
    for (int j = 0; j < 5; ++j) {
      for (const int keycode :
           {KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_1, KEY_2}) {
        remapper.Process(keycode, 1);
        remapper.Process(keycode, 2);
      }
    }
    remapper.Process(KEY_LEFTSHIFT, 1);
    for (const int keycode : {KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_1,
                              KEY_2, KEY_ESC, KEY_X}) {
      remapper.Process(keycode, 1);
      remapper.Process(keycode, 2);
    }
    remapper.Process(KEY_LEFTSHIFT, 0);
  }

  return 0;
}
