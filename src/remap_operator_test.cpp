#include "remap_operator.h"

#include <linux/input-event-codes.h>
#include <stdlib.h>

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "keycode_lookup.h"

// Helper functions.
std::string Join(const std::vector<std::string>& vec) {
  const std::string delimiter = ", ";
  std::ostringstream oss;
  oss << "[";
  for (size_t i = 0; i < vec.size(); ++i) {
    if (i != 0) {
      oss << delimiter;
    }
    oss << "\"" << vec[i] << "\"";
  }
  oss << "]";
  return oss.str();
}

std::vector<std::string> GetOutcomes(Remapper& remapper,
                                     std::vector<int> keycodes) {
  std::vector<std::string> outcomes;
  auto process = [&outcomes, &remapper](int keycode) {
    std::ostringstream oss;
    oss << "> " << (keycode > 0 ? "P " : "R ") << keyCodeToString(abs(keycode));
    outcomes.push_back(oss.str());
    remapper.process(keycode);
  };

  remapper.SetCallback([&outcomes](int keycode, int press) {
    std::ostringstream oss;
    oss << ". " << (press == 1 ? "P " : "R ") << keyCodeToString(keycode);
    outcomes.push_back(oss.str());
  });

  for (int keycode : keycodes) {
    process(keycode);
  }

  return outcomes;
}

bool AssertEqual(std::vector<std::string> obtained,
                 std::vector<std::string> expected) {
  if (obtained != expected) {
    printf("FAILED\n");
    std::cout << "obtained: " << Join(obtained) << std::endl;
    std::cout << "expected: " << Join(expected) << std::endl;
    return false;
  }
  printf("Passed.\n");
  return true;
}

// Tests.
bool Test1() {
  Remapper remapper;

  remapper.add_mapping("fnkeys", KEY_A, {remapper.action_key(KEY_B)});
  remapper.add_mapping("fnkeys", -KEY_A, {remapper.action_key(-KEY_B)});
  remapper.add_mapping("fnkeys", KEY_1, {remapper.action_key(KEY_F1)});
  remapper.add_mapping("fnkeys", KEY_0, {remapper.action_key(KEY_F10)});
  remapper.add_mapping("", KEY_RIGHTCTRL,
                       {remapper.action_key(KEY_RIGHTCTRL),
                        remapper.action_activate_mapping("fnkeys")});

  auto outcomes = GetOutcomes(remapper, {KEY_C, -KEY_C, KEY_RIGHTCTRL, KEY_A,
                                         -KEY_RIGHTCTRL, KEY_A, -KEY_A});

  std::vector<std::string> expected = {
      "> P KEY_C",         ". P KEY_C",         "> R KEY_C", ". R KEY_C",
      "> P KEY_RIGHTCTRL", ". P KEY_RIGHTCTRL", "> P KEY_A", ". P KEY_B",
      "> R KEY_RIGHTCTRL", ". R KEY_RIGHTCTRL", "> P KEY_A", ". P KEY_A",
      "> R KEY_A",        ". R KEY_A",
  };
  return AssertEqual(outcomes, expected);
}

int main() {
  bool all_passed = Test1();
  return all_passed ? 0 : 1;
}