#include "remap_operator.h"

#include <linux/input-event-codes.h>
#include <stdlib.h>

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "keycode_lookup.h"

int main() {
  std::vector<std::string> outcomes;
  auto dummy_emit_fn = [&outcomes](int keycode, int press) {
    std::ostringstream oss;
    oss << "  " << (press == 1 ? "P " : "R ") << keyCodeToString(keycode);
    outcomes.push_back(oss.str());
  };
  Remapper remapper(dummy_emit_fn);

  remapper.add_mapping("fnkeys", KEY_A, {remapper.action_key(KEY_B)});
  remapper.add_mapping("fnkeys", -KEY_A, {remapper.action_key(-KEY_B)});
  remapper.add_mapping("fnkeys", KEY_1, {remapper.action_key(KEY_F1)});
  remapper.add_mapping("fnkeys", KEY_0, {remapper.action_key(KEY_F10)});
  remapper.add_mapping("", KEY_RIGHTCTRL,
                       {remapper.action_key(KEY_RIGHTCTRL),
                        remapper.action_activate_mapping("fnkeys")});

  auto process = [&outcomes, &remapper](int keycode) {
    std::ostringstream oss;
    oss << (keycode > 0 ? "P " : "R ") << keyCodeToString(abs(keycode));
    remapper.process(keycode);
    outcomes.push_back(oss.str());
  };
  process(KEY_C);
  process(-KEY_C);
  // Should result in PCtrl RCtrl PB RB.
  process(KEY_RIGHTCTRL);
  process(KEY_A);
  // process(-KEY_A);
  process(-KEY_RIGHTCTRL);
  process(KEY_A);
  process(-KEY_A);

  for (const std::string line : outcomes) {
    std::cout << line << std::endl;
  }
  return 0;
}
