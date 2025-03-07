/*
 * Copyright (c) 2024 Nomen Aliud (aka Arnab Bose)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __REMAP_OPERATOR_H
#define __REMAP_OPERATOR_H

// This is responsible for loading and acting on remap configs.
// I.e., this is the brain of the remapper.
//
// WIP.
// TODO -
// - Implement json config parsing.
// - Need to handle repeats. 1 is press. 0 is release. And repeat is code 2.

#include <cstddef>
#include <functional>
#include <optional>
#include <stack>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "keycode_lookup.h"

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

  friend std::ostream& operator<<(std::ostream& os, const KeyEvent& key_event) {
    os << "(" << KeyCodeToName(key_event.key_code) << " ";
    switch (key_event.value) {
      case KeyEventType::kKeyPress:
        os << "Press";
        break;
      case KeyEventType::kKeyRelease:
        os << "Release";
        break;
      case KeyEventType::kKeyRepeat:
        os << "Repeat";
        break;
    }
    os << ")";
    return os;
  }

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
KeyEvent KeyPressEvent(int key_code);

KeyEvent KeyReleaseEvent(int key_code);

// Actions.
// In addition to below, there is KeyEvent which can directly act as an action.
struct ActionLayerChange {
  int layer_index;
};

struct ActionWait {
  int milli_seconds;
};

using Action = std::variant<KeyEvent, ActionLayerChange, ActionWait>;

// KeyEvent to which action they are mapped.
using ActionMap = std::unordered_map<KeyEvent, std::vector<Action>,
                                     KeyEvent::Hash, KeyEvent::Equal>;

// Encapsulates the state in which the mapper is right now.
// Layers are a kind of state.
// This has three major components -
// 1. ActionMap key - What events are we looking for.
// 2. ActionMap value - What do we do when that event occurs.
// 3. State parameters such as allow_other_keys enabled or not.
struct KeyboardState {
  ActionMap action_map;
  // If true, keys not in ActionMap are allowed.
  bool allow_other_keys = true;
  // If no interesting event such as keypress occurs while in this state, null
  // events are activated.
  std::vector<Action> null_event_actions;

  // Following are internal state, maintained by the remapper.

  bool null_event_applicable;

  // Called before activation. Activation is ignored if returns false.
  [[nodiscard]] bool activate() {
    if (is_active_) {
      std::cerr
          << "WARNING: Attempt to activate an already active layer. Denied."
          << std::endl;
      return false;
    }
    // std::cerr << "Activated" << this << "\n";
    is_active_ = true;
    null_event_applicable = true;
    return true;
  }
  // Called on deactivation.
  void deactivate() {
    // std::cerr << "Deactivated" << this << "\n";
    is_active_ = false;
  }

 private:
  // Double activation is not expected.
  // The error below makes complexity of managing is_active_ is justified,
  // because it will help us catch unexpected double activations.
  // E.g. due to logical errors, or due to things like the same key pressed
  // twice (though reasonably that seems impossible).
  bool is_active_;
};

class Remapper {
 public:
  Remapper();

  // Movable but not copyable.
  Remapper(Remapper&& other) = default;
  Remapper& operator=(Remapper&& other) = default;

  void SetCallback(std::function<void(int, int)> emit_key_code);

  // Default state_name is "".
  void AddMapping(const std::string& state_name, KeyEvent key_event,
                  const std::vector<Action>& actions);

  void SetNullEventActions(const std::string& state_name,
                           const std::vector<Action> actions);

  void SetAllowOtherKeys(const std::string& state_name, bool allow_other_keys);

  // Returns an action to activate a state. Can be part of actions in
  // AddMapping().
  ActionLayerChange ActionActivateState(std::string state_name);

  void Process(const int key_code_int, const int value);

  // Prints the existing config to terminal.
  void DumpConfig(std::ostream& os = std::cout) const;

 private:
  // Finds index of keyboard_state name. If it doesn't exist, adds it.
  int StateNameToIndex(const std::string& state_name);

  void EmitKeyCode(const KeyEvent& key_event);

  // Check if any layer was activated by the current key_code, and if so,
  // deactivate it.
  bool DeactivateLayerByKey(const KeyEvent& key_event);

  void DeactivateNLayers(const int n);

  void ProcessKeyEvent(const KeyEvent& key_event);

  // Expands an user-keypress into actions to be processed.
  const std::vector<Action> ExpandToActions(const KeyEvent& key_event) const;

  void ProcessActions(const std::vector<Action>& actions);

  void ProcessCombos(const KeyEvent& key_event);

  // TODO: Optimization to keep the active state updated in a variable?
  inline KeyboardState& active_state() {
    return active_layers_.size() > 0 ? *active_layers_.back().this_state
                                     : all_states_[0];
  }

  // These will be stored in a stack as new layers get activated.
  struct LayerActivation {
    int event_seq_num;         // When the layer was activated.
    const KeyEvent key_event;  // key_code that activated this layer.
    KeyboardState* this_state = nullptr;
  };

  // Do not use this directly, use StateNameToIndex().
  std::unordered_map<std::string, int> state_name_to_index_;

  // Index can be looked up from state name with StateToNameIndex().
  std::vector<KeyboardState> all_states_;

  // Previous mappings. This is used as mappings get deactivated.
  // Pair of key_code, mapping_index.
  std::vector<LayerActivation> active_layers_;

  // Current keys being held. Maps to event_seq_num, i.e. when it was held.
  // If somehow a key is pressed multiple times (e.g. repeats maybe?) then this
  // holds the last occurrence, as per the event_seq_num.
  struct KeyHeldInfo {
    // Which physical key resulted in this event.
    // Used while deactivation of a parent layer, to check if this key should
    // also be released.
    int key_origin;
    int event_seq_num;
  };
  std::unordered_map<int, KeyHeldInfo> keys_held_;

  // Can only increase.
  int event_seq_num_ = 0;

  // On Process(), key_codes are emitted via this callback.
  std::function<void(int, int)> emit_key_code_ = nullptr;

  // Progress to typing the kill combo.
  std::vector<int> combo_kill_keycodes_;
  std::size_t combo_kill_progress_ = 0;

  // The original key event being processed. Set on process().
  KeyEvent currently_processing_;
};

#endif  // __REMAP_OPERATOR_H