#ifndef __TEST_UTILS_H
#define __TEST_UTILS_H

#include <iostream>
#include <sstream>

#include "remap_operator.h"

using std::string;
using std::vector;

std::vector<string> GetOutcomes(Remapper& remapper, bool keep_incoming,
                                std::vector<std::pair<int, int>> keycodes) {
  std::vector<string> outcomes;
  auto process = [&outcomes, &remapper, keep_incoming](int keycode, int value) {
    if (keep_incoming) {
      std::ostringstream oss;
      oss << "In: ";
      if (value == 1) {
        oss << "P ";  // Press
      } else if (value == 0) {
        oss << "R ";  // Release
      } else if (value == 2) {
        oss << "T ";  // Repeat
      } else {
        throw std::runtime_error("Unexpected value");
      }
      oss << keyCodeToName(abs(keycode));
      outcomes.push_back(oss.str());
    }
    remapper.Process(keycode, value);
  };

  remapper.SetCallback([&outcomes](int keycode, int press) {
    std::ostringstream oss;
    std::string press_str;
    switch (press) {
      case 0:
        press_str = "R ";
        break;
      case 1:
        press_str = "P ";
        break;
      case 2:
        press_str = "T ";
        break;
      default:
        press_str = "U ";
        break;
    }
    oss << "Out: " << press_str << keyCodeToName(keycode);
    outcomes.push_back(oss.str());
  });

  for (const auto& [keycode, value] : keycodes) {
    process(keycode, value);
  }

  return outcomes;
}

#endif  // __TEST_UTILS_H