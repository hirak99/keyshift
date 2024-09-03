// This is responsible for loading and acting on remap configs.
// I.e., this is the brain of the remapper.
//
// WIP.
// TODO -
// - Implement json config parsing.
// - Need to handle repeats. 1 is press. 0 is release. And repeat is code 2.

#include <stdio.h>

#include <algorithm>
#include <functional>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// Note: Negative, -key_code is interpreted as key realease, both as condition
// and as an action.

// // Should change key_code to KeyEvent, which will contain the value.
// struct KeyEvent {
//   int key_code;
//   // 1 = key press
//   // 0 = key release
//   // 2 = key repeat
//   int value;
//   bool operator==(const KeyEvent& other) {
//     return key_code == other.key_code && value == other.value;
//   }
//   struct Equal {
//     bool operator()(const KeyEvent& lhs, const KeyEvent& rhs) const {
//       return lhs.key_code == rhs.key_code && lhs.value == rhs.value;
//     }
//   };
//   struct Hash {
//     std::size_t operator()(const KeyEvent& k) const {
//       std::size_t h1 = std::hash<int>{}(k.key_code);
//       std::size_t h2 = std::hash<int>{}(k.value);
//       return h1 ^ (h2 << 1);  // Combine hash values
//     }
//   };
// };

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
    if (deactivate_layer(key_code)) {
      return;
    }

    auto it = current_mapping_.find(key_code);
    if (it == current_mapping_.end()) {
      // Not found. Pass the key through by triggering as is and return.
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

  void emit_key_code(int key_code, bool press) {
    if (key_code < 0) {
      perror("WARNING: Negative key_code encountered in emit_key_code.");
      return;
    }
    if (emit_key_code_ != nullptr) {
      emit_key_code_(abs(key_code), press ? 1 : 0);
    }
    ++emitted_count_;
  }

  // Check if any layer was activated by the current key_code, and if so,
  // deactivate it.
  bool deactivate_layer(int key_code) {
    if (key_code > 0) return false;
    // Note: key_code is negative from now on.
    if (active_layers_.empty()) return false;
    if (active_layers_.top().key_code != -key_code) return false;

    auto layer_to_deactivate = active_layers_.top();
    active_layers_.pop();

    current_mapping_ = layer_to_deactivate.prior_mapping;
    // Get all the currently pressed keys after this was activated.
    int threshold = layer_to_deactivate.emitted_count;
    std::vector<KeyHeld> removed_keys;
    auto new_end =
        std::remove_if(keys_held_.begin(), keys_held_.end(),
                       [threshold, &removed_keys](const KeyHeld& kh) {
                         if (kh.emitted_count >= threshold) {
                           removed_keys.push_back(kh);
                           return true;
                         }
                         return false;
                       });
    // And release them.
    for (const auto& kh : removed_keys) {
      emit_key_code(kh.key_code, 0);
    }
    // And remove them from keys_held_.
    keys_held_.erase(new_end, keys_held_.end());
    trigger(key_code);
    return true;
  }

  void trigger(int key_code) {
    const bool press = key_code > 0;
    if (key_code < 0) key_code = -key_code;
    if (press) {
      keys_held_.push_back(KeyHeld{emitted_count_, key_code});
    } else {
      auto it = std::find_if(
          keys_held_.begin(), keys_held_.end(),
          [key_code](const KeyHeld& kh) { return kh.key_code == key_code; });
      if (it == keys_held_.end()) {
        // This key is not held. This is normal, and can happen when a lead key
        // is released if it was not set up to register a press.
        return;
      }
      keys_held_.erase(std::remove_if(
          keys_held_.begin(), keys_held_.end(),
          [key_code](KeyHeld& kh) { return kh.key_code == key_code; }));
    }
    emit_key_code(key_code, press);
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
