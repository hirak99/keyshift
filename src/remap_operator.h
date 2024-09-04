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
#include <iostream>
#include <optional>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

// Note: Negative, -key_code is interpreted as key realease, both as condition
// and as an action.

// Maps to the same codes as input.h value.
enum class KeyEventType {
  kKeyPress = 1,
  kKeyRelease = 0,
  kKeyRepeat = 2,
};

// Should change key_code to KeyEvent, which will contain the value.
struct KeyEvent {
  int key_code;
  KeyEventType value;

  // Following methods are needed to use this struct as key in map.
  bool operator==(const KeyEvent& other) const {
    return key_code == other.key_code && value == other.value;
  }
  bool operator!=(const KeyEvent& other) const {
    return key_code != other.key_code || value != other.value;
  }
  struct Equal {
    bool operator()(const KeyEvent& lhs, const KeyEvent& rhs) const {
      return lhs.key_code == rhs.key_code && lhs.value == rhs.value;
    }
  };
  struct Hash {
    std::size_t operator()(const KeyEvent& k) const {
      std::size_t h1 = std::hash<int>{}(k.key_code);
      std::size_t h2 = std::hash<int>{}(int(k.value));
      return h1 ^ (h2 << 1);  // Combine hash values
    }
  };
};

// Public
KeyEvent KeyPressEvent(int key_code) {
  return KeyEvent{key_code, KeyEventType::kKeyPress};
}

KeyEvent KeyReleaseEvent(int key_code) {
  return KeyEvent{key_code, KeyEventType::kKeyRelease};
}

// Actions.
// In addition to below, there is KeyEvent which can directly act as an action.
struct ActionLayerChange {
  int layer_index;
};

using Action = std::variant<KeyEvent, ActionLayerChange>;

// KeyEvent to which action they are mapped.
using ActionMap = std::unordered_map<KeyEvent, std::vector<Action>,
                                     KeyEvent::Hash, KeyEvent::Equal>;

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
  void add_mapping(const std::string& name, KeyEvent key_event,
                   const std::vector<Action>& actions) {
    auto& mapping = all_mappings_[get_mapping_number(name)];
    mapping[key_event] = actions;
  }

  ActionLayerChange action_activate_mapping(std::string mapping_name) {
    return ActionLayerChange{get_mapping_number(mapping_name)};
  }

  void process(int key_code_int, int value) {
    KeyEvent key_event{key_code_int, KeyEventType(value)};
    // Check if key_event is in activated mapping stack.
    if (deactivate_layer(key_event)) {
      return;
    }

    auto it = current_mapping_.find(key_event);
    if (it == current_mapping_.end()) {
      // Not remapped. Pass thru.
      process_key_event(key_event);
      return;
    }
    for (const Action& action : it->second) {
      if (std::holds_alternative<KeyEvent>(action)) {
        process_key_event(std::get<KeyEvent>(action));
      } else if (std::holds_alternative<ActionLayerChange>(action)) {
        const auto& layer_change = std::get<ActionLayerChange>(action);
        auto it = all_mappings_.find(layer_change.layer_index);
        if (it != all_mappings_.end()) {
          active_layers_.push(
              LayerActivation{event_seq_num_++, key_event, current_mapping_});
          current_mapping_ = it->second;
        } else {
          perror(
              "WARNING: Invalid mapping code. This is unexpected, please "
              "report a bug.");
        }
      } else {
        perror("WARNING: Unknown action.");
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

  void emit_key_code(KeyEvent key_event) {
    // std::cout << "Emit "
    //           << (key_event.value == KeyEventType::kKeyPress ? "P" : "R")
    //           << key_event.key_code << std::endl;
    if (emit_key_code_ != nullptr) {
      emit_key_code_(key_event.key_code, int(key_event.value));
    }
  }

  // Check if any layer was activated by the current key_code, and if so,
  // deactivate it.
  bool deactivate_layer(KeyEvent key_event) {
    if (active_layers_.empty()) return false;
    // TODO: The key_event may correspondo to not the top layer. Change this to
    // do a search on all active layers.
    auto layer_to_deactivate = active_layers_.top();
    if (layer_to_deactivate.key_event.key_code != key_event.key_code)
      return false;

    active_layers_.pop();

    current_mapping_ = layer_to_deactivate.prior_mapping;
    // Get all the currently pressed keys after this was activated.
    int threshold = layer_to_deactivate.event_seq_num;
    std::vector<KeyHeld> removed_keys;
    auto new_end =
        std::remove_if(keys_held_.begin(), keys_held_.end(),
                       [threshold, &removed_keys](const KeyHeld& kh) {
                         if (kh.event_seq_num >= threshold) {
                           removed_keys.push_back(kh);
                           return true;
                         }
                         return false;
                       });
    // And release them, in reverse order.
    for(auto it=removed_keys.rbegin(); it != removed_keys.rend(); ++it) {
      emit_key_code(KeyEvent{it->key_code, KeyEventType::kKeyRelease});
    }
    // And remove them from keys_held_.
    keys_held_.erase(new_end, keys_held_.end());
    process_key_event(key_event);
    return true;
  }

  void process_key_event(KeyEvent key_event) {
    if (key_event.value == KeyEventType::kKeyPress) {
      keys_held_.push_back(KeyHeld{event_seq_num_++, key_event.key_code});
    } else if (key_event.value == KeyEventType::kKeyRelease) {
      auto it = std::find_if(keys_held_.begin(), keys_held_.end(),
                             [key_event](const KeyHeld& kh) {
                               return kh.key_code == key_event.key_code;
                             });
      if (it == keys_held_.end()) {
        // This key is not actually held. This is normal, and can happen when a
        // lead key is released if it was not set up to register a press.
        return;
      }
      keys_held_.erase(std::remove_if(
          keys_held_.begin(), keys_held_.end(), [key_event](KeyHeld& kh) {
            return kh.key_code == key_event.key_code;
          }));
    } else {
      std::cerr << "WARNING: Unimplemented key code value "
                << int(key_event.value) << std::endl;
    }
    emit_key_code(key_event);
  }

  // These will be stored in a stack as new layers get activated.
  struct LayerActivation {
    int event_seq_num;              // When the layer was activated.
    KeyEvent key_event;             // key_code that activated this layer.
    const ActionMap prior_mapping;  // Mapping to revert to on deactivation.
  };

  // A key that was pressed. All currently held keys will be kept in a vector.
  struct KeyHeld {
    int event_seq_num;  // When the key was held.
    int key_code;       // key_code that was pressed.
  };

  // Do not use this directly, use get_mapping_number().
  std::unordered_map<std::string, int> mapping_name_to_index_;

  // Outer key is a lookup from the mapping name.
  // Inner key is the condition, a key_code. Negative indicates on release.
  std::unordered_map<int, ActionMap> all_mappings_;

  // Currently activated mapping. Maintained for efficiency during operation.
  ActionMap& current_mapping_;
  // Previous mappings. This is used as mappings get deactivated.
  // Pair of key_code, mapping_index.
  std::stack<LayerActivation> active_layers_;

  // Current keys being pressed. This is based on what's emitted.
  std::vector<KeyHeld> keys_held_;

  // Can only increase.
  int event_seq_num_ = 0;

  // On process(), key_codes are emitted via this callback.
  std::function<void(int, int)> emit_key_code_ = nullptr;
};
