#include "keycode_lookup.h"

std::shared_ptr<KeyCodes> KeyCodes::instance_ = nullptr;

// For convenience.

std::string KeyCodeToName(int key_code) {
  return KeyCodes::instance()->toString(key_code);
}

std::optional<int> NameToKeyCode(std::string name) {
  return KeyCodes::instance()->toCode(name);
}
