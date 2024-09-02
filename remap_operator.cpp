// This is responsible for loading and acting on remap configs.
// I.e., this is the brain of the remapper.
// WIP.
#include <linux/input-event-codes.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "keycode_lookup.h"

// Note: -key_code is interpreted as key realease, both as condition and as action.

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
  Remapper() : current_mapping_(all_mappings_[0]) {
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
    const std::string press = (keycode > 0 ? "Press" : "Release");
    if (keycode < 0) keycode = -keycode;
    std::cout << keyCodeToString(keycode) << " " << press << std::endl;
  }

  // For use while constructing the mapping only. Not used during processing.
  std::unordered_map<std::string, int> mapping_name_to_index_;

  // Outer key is a lookup from the mapping name.
  // Inner key is the condition, a keycode. Negative indicates on release.
  std::unordered_map<int, std::unordered_map<int, std::vector<Action>>>
      all_mappings_;
  std::unordered_map<int, std::vector<Action>>& current_mapping_;
};

int main() {
  Remapper remapper;
  remapper.add_mapping("", KEY_A, {Action{ActionType::kKeyEvent, KEY_B}});
  remapper.add_mapping("", -KEY_A, {Action{ActionType::kKeyEvent, -KEY_B}});
  remapper.process(KEY_B);
  remapper.process(KEY_A);
  remapper.process(-KEY_A);
  return 0;
}
