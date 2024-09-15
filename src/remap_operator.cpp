#include "remap_operator.h"

#include <stdio.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <stack>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "essentials.h"

KeyEvent KeyPressEvent(int key_code) {
  return KeyEvent{key_code, KeyEventType::kKeyPress};
}

KeyEvent KeyReleaseEvent(int key_code) {
  return KeyEvent{key_code, KeyEventType::kKeyRelease};
}

Remapper::Remapper() : default_state_(all_states_[0]) {
  // Make "" to have index 0.
  if (StateNameToIndex("") != 0) {
    // Should not happen!
    throw std::runtime_error("Could not assert first index to be 0");
  }
}

void Remapper::SetCallback(std::function<void(int, int)> emit_key_code) {
  emit_key_code_ = emit_key_code;
}

// Default state_name is "".
void Remapper::AddMapping(const std::string& state_name, KeyEvent key_event,
                          const std::vector<Action>& actions) {
  auto& keyboard_state = all_states_[StateNameToIndex(state_name)];

  // If exists, append. Else set.
  auto it = keyboard_state.action_map.find(key_event);
  if (it != keyboard_state.action_map.end()) {
    it->second.insert(it->second.end(), actions.begin(), actions.end());
  } else {
    keyboard_state.action_map[key_event] = actions;
  }

  // Do not auto deactivate if there is any explicit deactivation request.
  if (std::any_of(actions.begin(), actions.end(), [](Action action) {
        return std::holds_alternative<ActionDeactivateLayer>(action);
      })) {
    keyboard_state.auto_deactivate_layer = false;
  }
}

void Remapper::SetNullEventActions(const std::string& state_name,
                                   const std::vector<Action> actions) {
  auto& keyboard_state = all_states_[StateNameToIndex(state_name)];
  keyboard_state.null_event_actions = actions;
}

void Remapper::SetAllowOtherKeys(const std::string& state_name,
                                 bool allow_other_keys) {
  auto& keyboard_state = all_states_[StateNameToIndex(state_name)];
  keyboard_state.allow_other_keys = allow_other_keys;
}

ActionLayerChange Remapper::ActionActivateState(std::string state_name) {
  return ActionLayerChange{StateNameToIndex(state_name)};
}

std::vector<Action> Remapper::ExpandToActions(const KeyEvent& key_event) {
  // Iterate: active_layers_.reverse() + {default_state_}.
  for (int layer_index = active_layers_.size() - 1; layer_index >= -1;
       --layer_index) {
    const auto& this_state = layer_index >= 0
                                 ? active_layers_[layer_index].this_state
                                 : default_state_;
    auto it = this_state.action_map.find(key_event);
    if (it != this_state.action_map.end()) {
      // Return the remapped actions.
      return it->second;
    }

    // Not remapped but all other keys not allowed.
    if (!this_state.allow_other_keys) {
      return {};
    }
  }
  return {key_event};
}

void Remapper::Process(int key_code_int, int value) {
  KeyEvent key_event{key_code_int, KeyEventType(value)};
  // Check if key_event is in activated keyboard_state stack.
  if (DeactivateLayerByKey(key_event)) {
    return;
  }

  // Since a key was pressed, null event will not be triggered on
  // deactivation.
  active_state().null_event_applicable = false;

  ProcessActions(ExpandToActions(key_event), key_event);
}

void Remapper::DumpConfig(std::ostream& os) const {
  for (const auto& [id, state] : Sorted(all_states_)) {
    os << "State #" << id << std::endl;
    os << "  Other keys: " << (state.allow_other_keys ? "Allow" : "Block")
       << std::endl;
    auto ShowActions = [&os](const std::vector<Action>& actions) {
      for (const auto& action : actions) {
        if (std::holds_alternative<KeyEvent>(action)) {
          const auto& key_event = std::get<KeyEvent>(action);
          os << "    Key: " << key_event << std::endl;
        } else if (std::holds_alternative<ActionLayerChange>(action)) {
          const auto& layer_change = std::get<ActionLayerChange>(action);
          os << "    Layer Change: " << layer_change.layer_index << std::endl;
        } else if (std::holds_alternative<ActionDeactivateLayer>(action)) {
          os << "    Deactivate Layer" << std::endl;
        } else {
          perror("WARNING: Unknown action.");
        }
      }
    };
    for (const auto& [trigger, actions] : state.action_map) {
      os << "  On: " << trigger << std::endl;
      ShowActions(actions);
    }
    if (!state.null_event_actions.empty()) {
      os << "  On nothing:" << std::endl;
      ShowActions(state.null_event_actions);
    }
  }
}

// PRIVATE

// Finds index of keyboard_state name. If it doesn't exist, adds it.
int Remapper::StateNameToIndex(std::string state_name) {
  auto result = MapLookup(state_name_to_index_, state_name);
  if (result.has_value()) return *result;

  int index = state_name_to_index_.size();
  state_name_to_index_.emplace(state_name, index);
  return index;
}

void Remapper::EmitKeyCode(KeyEvent key_event) {
  // std::cout << "Emit "
  //           << (key_event.value == KeyEventType::kKeyPress ? "P" : "R")
  //           << key_event.key_code << std::endl;
  if (emit_key_code_ != nullptr) {
    emit_key_code_(key_event.key_code, int(key_event.value));
  }
}

// Check if any layer was activated by the current key_code, and if so,
// deactivate it.
bool Remapper::DeactivateLayerByKey(KeyEvent key_event) {
  if (key_event.value != KeyEventType::kKeyRelease) return false;
  if (active_layers_.empty()) return false;
  // TODO: The key_event may not correspond to the top layer. Change this to
  // do a search on all active layers.
  auto layer_to_deactivate = active_layers_.back();
  if (layer_to_deactivate.key_event.key_code != key_event.key_code)
    return false;

  DeactivateCurrentLayer();
  ProcessKeyEvent(key_event);
  return true;
}

void Remapper::DeactivateCurrentLayer() {
  if (active_layers_.empty()) {
    perror("WARNING: Trying to deactivate when no layer is active.");
    return;
  }
  auto layer_to_deactivate = active_layers_.back();
  active_layers_.pop_back();

  auto state_to_deactivate = layer_to_deactivate.this_state;
  state_to_deactivate.deactivate();
  if (state_to_deactivate.null_event_applicable) {
    ProcessActions(state_to_deactivate.null_event_actions, std::nullopt);
  }
  // Get all the currently pressed keys after this was activated.
  int threshold = layer_to_deactivate.event_seq_num;
  std::vector<std::pair<int, int>> removed_keys;
  // Erase keys held after the layer was activated.
  for (auto it = keys_held_.begin(); it != keys_held_.end();) {
    if (it->second > threshold) {
      removed_keys.push_back({it->first, it->second});
      it = keys_held_.erase(it);
    } else {
      ++it;
    }
  }
  // And release them in reverse order.
  std::sort(removed_keys.begin(), removed_keys.end(),
            [](const std::pair<int, int>& lhs, const std::pair<int, int>& rhs) {
              if (lhs.second != rhs.second) {
                return lhs.second < rhs.second;
              }
              return lhs.first < rhs.first;
            });
  for (auto it = removed_keys.rbegin(); it != removed_keys.rend(); ++it) {
    EmitKeyCode(KeyEvent{it->first, KeyEventType::kKeyRelease});
  }
}

void Remapper::ProcessKeyEvent(KeyEvent key_event) {
  if (key_event.value == KeyEventType::kKeyPress) {
    keys_held_[key_event.key_code] = event_seq_num_++;
    EmitKeyCode(key_event);
  } else if (key_event.value == KeyEventType::kKeyRepeat) {
    // If the repeating key was used to activate a layer, do nothing.
    for (const auto& layer : active_layers_) {
      if (layer.key_event.key_code == key_event.key_code) return;
    }
    // Else, emit the key.
    EmitKeyCode(key_event);
  } else if (key_event.value == KeyEventType::kKeyRelease) {
    if (!MapContains(keys_held_, key_event.key_code)) {
      // This key is not actually held. This is normal, and can happen when a
      // lead key is released if it was not set up to register a press.
      return;
    }
    keys_held_.erase(key_event.key_code);
    EmitKeyCode(key_event);
  } else {
    std::cerr << "WARNING: Unimplemented key code value "
              << int(key_event.value) << std::endl;
    return;
  }
}

void Remapper::ProcessActions(const std::vector<Action>& actions,
                              const std::optional<KeyEvent> key_event) {
  for (const Action& action : actions) {
    if (std::holds_alternative<KeyEvent>(action)) {
      ProcessKeyEvent(std::get<KeyEvent>(action));
    } else if (std::holds_alternative<ActionLayerChange>(action)) {
      const auto& layer_change = std::get<ActionLayerChange>(action);
      auto it = all_states_.find(layer_change.layer_index);
      if (it != all_states_.end()) {
        auto new_state = it->second;
        if (new_state.activate()) {
          if (new_state.auto_deactivate_layer && key_event.has_value()) {
            active_layers_.push_back(
                LayerActivation{event_seq_num_++, *key_event, new_state});
          }
        }
      } else {
        perror(
            "WARNING: Invalid keyboard_state code. This is unexpected, please "
            "report a bug.");
      }
    } else if (std::holds_alternative<ActionDeactivateLayer>(action)) {
      DeactivateCurrentLayer();
    } else {
      perror("WARNING: Unknown action.");
    }
  }
}
