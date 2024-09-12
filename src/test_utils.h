#ifndef __TEST_UTILS_H
#define __TEST_UTILS_H

#include <iostream>
#include <sstream>

#include "remap_operator.h"

using std::string;
using std::vector;

vector<string> GetOutcomes(Remapper& remapper, bool keep_incoming,
                           vector<int> keycodes) {
  vector<string> outcomes;
  auto process = [&outcomes, &remapper, keep_incoming](int keycode) {
    if (keep_incoming) {
      std::ostringstream oss;
      oss << "In: " << (keycode > 0 ? "P " : "R ")
          << keyCodeToName(abs(keycode));
      outcomes.push_back(oss.str());
    }
    remapper.Process(abs(keycode), keycode > 0 ? 1 : 0);
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

  for (int keycode : keycodes) {
    process(keycode);
  }

  return outcomes;
}

#endif  // __TEST_UTILS_H