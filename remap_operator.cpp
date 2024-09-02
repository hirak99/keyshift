// This is responsible for loading and acting on remap configs.
// I.e., this is the brain of the remapper.
//
// WIP.
// TODO -
// - Implement context switching.
// - Implement json config parsing.

#include <linux/input-event-codes.h>

#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "keycode_lookup.h"

// Note: -key_code is interpreted as key realease, both as condition and as an
// action.

enum class ActionType {
  kKeyEvent,
  kChangeMapping,
};

struct Action {
  ActionType type;
  // For kKeyEvent, this is the +/- keycode. If negative, it is a release event.
  // For kChangeMapping, this is the mapping index to switch to.
  int argument;
};

class Remapper {
 public:
  Remapper(std::function<void(int, int)> emit_keycode)
      : current_mapping_(all_mappings_[0]), emit_keycode_(emit_keycode) {
    mapping_name_to_index_[""] = 0;
  }

  // Default table should have name "".
  void add_mapping(const std::string& name, int key_code,
                   const std::vector<Action>& actions) {
    auto& mapping = all_mappings_[mapping_name_to_index_[name]];
    mapping[key_code] = actions;
  }

  void process(int keycode) {
    auto it = current_mapping_.find(keycode);
    if (it == current_mapping_.end()) {
      trigger(keycode);
      return;
    }
    for (const Action& action : it->second) {
      if (action.type == ActionType::kKeyEvent) {
        trigger(action.argument);
      }
    }
  }

 private:
  void trigger(int keycode) {
    const int press = (keycode > 0 ? 1 : 0);
    if (keycode < 0) keycode = -keycode;
    emit_keycode_(keycode, press);
  }

  // For use while constructing the mapping only. Not used during processing.
  std::unordered_map<std::string, int> mapping_name_to_index_;

  // Outer key is a lookup from the mapping name.
  // Inner key is the condition, a keycode. Negative indicates on release.
  std::unordered_map<int, std::unordered_map<int, std::vector<Action>>>
      all_mappings_;
  std::unordered_map<int, std::vector<Action>>& current_mapping_;

  std::function<void(int, int)> emit_keycode_;
};

int main() {
  auto dummy_emit_fn = [](int keycode, int press) {
    std::cout << (press == 1 ? "Press " : "Release ")
              << keyCodeToString(keycode) << std::endl;
  };
  Remapper remapper(dummy_emit_fn);
  remapper.add_mapping("", KEY_A, {Action{ActionType::kKeyEvent, KEY_B}});
  remapper.add_mapping("", -KEY_A, {Action{ActionType::kKeyEvent, -KEY_B}});
  remapper.process(KEY_B);
  remapper.process(KEY_A);
  remapper.process(-KEY_A);
  return 0;
}
