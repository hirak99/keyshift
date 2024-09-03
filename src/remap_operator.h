// This is responsible for loading and acting on remap configs.
// I.e., this is the brain of the remapper.
//
// WIP.
// TODO -
// - Implement context switching.
// - Implement json config parsing.

#include <stdio.h>

#include <algorithm>
#include <functional>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// Note: -key_code is interpreted as key realease, both as condition and as an
// action.

enum class ActionType {
  kKeyEvent,
  kActivateMapping,
};

struct Action {
  ActionType type;
  // For kKeyEvent, this is the +/- key_code. If negative, it is a release
  // event. For kActivateMapping, this is the mapping index to switch to.
  int argument;
};

using ActionMap = std::unordered_map<int, std::vector<Action>>;

// These will be stored in a stack as new layers get activated.
struct LayerActivation {
  int emitted_count;  // When the layer was activated.
  int key_code;       // key_code that activated this layer. Only positive.
  const ActionMap prior_mapping;  // Mapping to revert to on deactivation.
};

// A key that was pressed. All currently held keys will be kept in a vector.
struct KeyHeld {
  int emitted_count;  // When the key was held.
  int key_code;       // key_code that was pressed. Only positive.
};

class Remapper {
 public:
  Remapper() : current_mapping_(all_mappings_[0]) {
    // Ensure that "" is 0.
    (void)get_mapping_number("");
  }

  void SetCallback(std::function<void(int, int)> emit_key_code) {
    emit_key_code_ = emit_key_code;
  }

  // Default table should have name "".
  void add_mapping(const std::string& name, int key_code,
                   const std::vector<Action>& actions) {
    auto& mapping = all_mappings_[get_mapping_number(name)];
    mapping[key_code] = actions;
  }

  Action action_key(int key_code) {
    return Action{ActionType::kKeyEvent, key_code};
  }

  Action action_activate_mapping(std::string mapping_name) {
    return Action{ActionType::kActivateMapping,
                  get_mapping_number(mapping_name)};
  }

  void process(int key_code) {
    // Check if key_code is in activated mapping stack.
    if (process_deactivation(key_code)) {
      return;
    }

    auto it = current_mapping_.find(key_code);
    if (it == current_mapping_.end()) {
      trigger(key_code);
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
            active_layers_.push(
                LayerActivation{emitted_count_, key_code, current_mapping_});
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

  bool process_deactivation(int key_code) {
    if (key_code > 0) return false;
    // Note: key_code is negative from now on.
    if (active_layers_.empty()) return false;
    if (active_layers_.top().key_code != -key_code) return false;

    current_mapping_ = active_layers_.top().prior_mapping;
    active_layers_.pop();
    // TODO: Release all the currently pressed keys after this was activated.
    trigger(key_code);
    return true;
  }

  void trigger(int key_code) {
    const int press = (key_code > 0 ? 1 : 0);
    if (key_code < 0) key_code = -key_code;
    if (press == 1) {
      keys_held_.push_back(KeyHeld{emitted_count_, key_code});
    } else {
      auto it = std::find_if(
          keys_held_.begin(), keys_held_.end(),
          [key_code](const KeyHeld& kh) { return kh.key_code == key_code; });
      if (it == keys_held_.end()) {
        // This key is not held.
        perror("WARNING: Ignoring key release event as key not held.");
        return;
      }
      keys_held_.erase(std::remove_if(
          keys_held_.begin(), keys_held_.end(),
          [key_code](KeyHeld& kh) { return kh.key_code == key_code; }));
    }
    if (emit_key_code_ != nullptr) {
      emit_key_code_(key_code, press);
    }
    ++emitted_count_;
  }

  // Do not use this directly, use get_mapping_number().
  std::unordered_map<std::string, int> mapping_name_to_index_;

  // Outer key is a lookup from the mapping name.
  // Inner key is the condition, a key_code. Negative indicates on release.
  std::unordered_map<int, ActionMap> all_mappings_;

  // Currently activated mapping.
  ActionMap& current_mapping_;
  // Previous mappings. This is used as mappings get deactivated.
  // Pair of key_code, mapping_index.
  std::stack<LayerActivation> active_layers_;

  // Current keys being pressed. This is based on what's emitted.
  std::vector<KeyHeld> keys_held_;

  // How many keys emitted so far. Acts as a convenient time tag for events.
  int emitted_count_ = 0;

  // On process(), key_codes are emitted via this callback.
  std::function<void(int, int)> emit_key_code_ = nullptr;
};
