// This is responsible for loading and acting on remap configs.
// I.e., this is the brain of the remapper.
//
// WIP.
// TODO -
// - Implement context switching.
// - Implement json config parsing.

#include <linux/input-event-codes.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

#include "keycode_lookup.h"

// Note: -key_code is interpreted as key realease, both as condition and as an
// action.

enum class ActionType {
  kKeyEvent,
  kActivateMapping,
  kDeactivateMapping,
};

struct Action {
  ActionType type;
  // For kKeyEvent, this is the +/- keycode. If negative, it is a release event.
  // For kActivateMapping, this is the mapping index to switch to.
  int argument;
};

using ActionMap = std::unordered_map<int, std::vector<Action>>;

class Remapper {
 public:
  Remapper(std::function<void(int, int)> emit_keycode)
      : current_mapping_(all_mappings_[0]), emit_keycode_(emit_keycode) {
    // Ensure that "" is 0.
    (void)get_mapping_number("");
  }

  // Default table should have name "".
  void add_mapping(const std::string& name, int key_code,
                   const std::vector<Action>& actions) {
    auto index_it = mapping_name_to_index_.find(name);
    auto& mapping = all_mappings_[get_mapping_number(name)];
    mapping[key_code] = actions;
  }

  Action action_key(int keycode) {
    return Action{ActionType::kKeyEvent, keycode};
  }

  Action action_activate_mapping(std::string mapping_name) {
    return Action{ActionType::kActivateMapping,
                  get_mapping_number(mapping_name)};
  }

  void process(int keycode) {
    // Check if keycode is in activated mapping stack.
    if (process_deactivation(keycode)) {
      return;
    }

    auto it = current_mapping_.find(keycode);
    if (it == current_mapping_.end()) {
      trigger(keycode);
      return;
    }
    for (const Action& action : it->second) {
      switch (action.type) {
        case ActionType::kKeyEvent:
          trigger(action.argument);
          break;
        case ActionType::kActivateMapping:
          auto it = all_mappings_.find(action.argument);
          if (it != all_mappings_.end()) {
            mappings_pushed_.push({-keycode, current_mapping_});
            current_mapping_ = it->second;
          } else {
            perror(
                "WARNING: Invalid mapping code. This is unexpected, please "
                "report a bug.");
          }
          break;
      }
    }
  }

 private:
  // Finds index of mapping name. If it doesn't exist, adds it.
  int get_mapping_number(std::string name) {
    auto index_it = mapping_name_to_index_.find(name);
    if (index_it == mapping_name_to_index_.end()) {
      int index = mapping_name_to_index_.size();
      mapping_name_to_index_[name] = index;
      return index;
    }
    return index_it->second;
  }

  bool process_deactivation(int keycode) {
    if (keycode > 0) return false;
    if (mappings_pushed_.empty()) return false;
    if (mappings_pushed_.top().first != keycode) return false;

    current_mapping_ = mappings_pushed_.top().second;
    mappings_pushed_.pop();
    // TODO: Send release keys for whatever was held after this key.
    trigger(keycode);
    return true;
  }

  void trigger(int keycode) {
    const int press = (keycode > 0 ? 1 : 0);
    if (keycode < 0) keycode = -keycode;
    if (press == 1) {
      keys_held_.push_back(keycode);
    } else {
      if (std::find(keys_held_.begin(), keys_held_.end(), keycode) ==
          keys_held_.end()) {
        // This key is not held.
        return;
      }
      keys_held_.erase(
          std::remove_if(keys_held_.begin(), keys_held_.end(),
                         [keycode](int x) { return x == keycode; }));
    }
    emit_keycode_(keycode, press);
  }

  // Do not use this directly, use get_mapping_number().
  std::unordered_map<std::string, int> mapping_name_to_index_;

  // Outer key is a lookup from the mapping name.
  // Inner key is the condition, a keycode. Negative indicates on release.
  std::unordered_map<int, ActionMap> all_mappings_;

  // Currently activated mapping.
  ActionMap& current_mapping_;
  // Previous mappings. This is used as mappings get deactivated.
  // Pair of keycode, mapping_index.
  std::stack<std::pair<int, const ActionMap>> mappings_pushed_;

  // Current keys being held. This is based on what's emitted.
  // Contains only positive values.
  std::vector<int> keys_held_;

  // On process(), keycodes are emitted via this callback.
  std::function<void(int, int)> emit_keycode_;
};

int main() {
  auto dummy_emit_fn = [](int keycode, int press) {
    std::cout << "  " << (press == 1 ? "P " : "R ") << keyCodeToString(keycode)
              << std::endl;
  };
  Remapper remapper(dummy_emit_fn);

  remapper.add_mapping("fnkeys", KEY_A, {remapper.action_key(KEY_B)});
  remapper.add_mapping("fnkeys", -KEY_A, {remapper.action_key(-KEY_B)});
  remapper.add_mapping("fnkeys", KEY_1, {remapper.action_key(KEY_F1)});
  remapper.add_mapping("fnkeys", KEY_0, {remapper.action_key(KEY_F10)});
  remapper.add_mapping("", KEY_RIGHTCTRL,
                       {remapper.action_key(KEY_RIGHTCTRL),
                        remapper.action_activate_mapping("fnkeys")});

  auto process = [&remapper](int keycode) {
    std::cout << (keycode > 0 ? "P " : "R ") << keyCodeToString(abs(keycode))
              << std::endl;
    remapper.process(keycode);
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
  return 0;
}
