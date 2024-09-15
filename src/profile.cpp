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

std::vector<string> SplitLines(const string& str) {
  std::vector<string> lines;
  string line;
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

  for (int i = 0; i < 1000000; ++i) {
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