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

#ifndef __KEYCODE_LOOKUP_H
#define __KEYCODE_LOOKUP_H

#include <iostream>
#include <memory>
#include <optional>

#include "keycodes.h"

class KeyCodes {
 public:
  static const std::shared_ptr<KeyCodes> instance() {
    if (instance_ == nullptr) {
      instance_.reset(new KeyCodes());
    }
    return instance_;
  }

  KeyCodes(const KeyCodes&) = delete;
  KeyCodes& operator=(const KeyCodes&) = delete;

  std::string toString(int key_code) const {
    auto it = keycode_to_name_.find(key_code);
    if (it == keycode_to_name_.end()) {
      return "UNRECOGNIZED_KEY_CODE(" + std::to_string(key_code) + ")";
    }
    return it->second;
  }

  std::optional<int> toCode(std::string name) const {
    auto it = kKeyCodeNames.find(name);
    if (it == kKeyCodeNames.end()) {
      std::cerr << "keycode_lookup: Unknown key " << name << std::endl;
      return std::nullopt;
    }
    return it->second;
  }

 private:
  KeyCodes() {
    for (const auto& pair : kKeyCodeNames) {
      keycode_to_name_[pair.second] = pair.first;
    }
  }
  static std::shared_ptr<KeyCodes> instance_;

  std::unordered_map<int, std::string> keycode_to_name_;
};

// For convenience.

std::string KeyCodeToName(int key_code);

std::optional<int> NameToKeyCode(std::string name);

#endif  // __KEYCODE_LOOKUP_H
