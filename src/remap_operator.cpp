#include "remap_operator.h"

#include <stdio.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

KeyEvent KeyPressEvent(int key_code) {
  return KeyEvent{key_code, KeyEventType::kKeyPress};
}

KeyEvent KeyReleaseEvent(int key_code) {
  return KeyEvent{key_code, KeyEventType::kKeyRelease};
}

Remapper::Remapper() : current_state_(all_states_[0]) {
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

void Remapper::Process(int key_code_int, int value) {
  KeyEvent key_event{key_code_int, KeyEventType(value)};
  // Check if key_event is in activated keyboard_state stack.
  if (DeactivateLayerByKey(key_event)) {
    return;
  }

  auto it = current_state_.action_map.find(key_event);
  if (it == current_state_.action_map.end()) {
    // Not remapped.
    if (current_state_.allow_other_keys) {
      ProcessKeyEvent(key_event);
    }
    return;
  }
  // Since a key was pressed, null event will not be triggered on
  // deactivation.
  current_state_.null_event_applicable = false;
  ProcessActions(it->second, key_event);
}

void Remapper::DumpConfig(std::ostream& os) const {
  for (const auto& [id, state] : all_states_) {
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
  auto index_it = state_name_to_index_.find(state_name);
  if (index_it == state_name_to_index_.end()) {
    int index = state_name_to_index_.size();
    state_name_to_index_[state_name] = index;
    return index;
  }
  return index_it->second;
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
  if (active_layers_.empty()) return false;
  // TODO: The key_event may not correspond to the top layer. Change this to
  // do a search on all active layers.
  auto layer_to_deactivate = active_layers_.top();
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
  auto layer_to_deactivate = active_layers_.top();
  active_layers_.pop();

  current_state_.deactivate();
  if (current_state_.null_event_applicable) {
    ProcessActions(current_state_.null_event_actions, std::nullopt);
  }
  current_state_ = layer_to_deactivate.prior_state;
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
  } else if (key_event.value == KeyEventType::kKeyRelease) {
    auto it = keys_held_.find(key_event.key_code);
    if (it == keys_held_.end()) {
      // This key is not actually held. This is normal, and can happen when a
      // lead key is released if it was not set up to register a press.
      return;
    }
    keys_held_.erase(key_event.key_code);
  } else {
    std::cerr << "WARNING: Unimplemented key code value "
              << int(key_event.value) << std::endl;
  }
  EmitKeyCode(key_event);
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
            active_layers_.push(
                LayerActivation{event_seq_num_++, *key_event, current_state_});
          }
          current_state_ = new_state;
        }
      } else {
        perror(
            "WARNING: Invalid keyboard_state code. This is unexpected, "
            "please "
            "report a bug.");
      }
    } else if (std::holds_alternative<ActionDeactivateLayer>(action)) {
      DeactivateCurrentLayer();
    } else {
      perror("WARNING: Unknown action.");
    }
  }
}
