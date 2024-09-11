#ifndef __KEYCODE_LOOKUP_H
#define __KEYCODE_LOOKUP_H

#include <linux/input-event-codes.h>

#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>

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
    auto it = name_to_keycode_.find(name);
    if (it == name_to_keycode_.end()) {
      std::cerr << "keycode_lookup: Unknown key " << name << std::endl;
      return std::nullopt;
    }
    return it->second;
  }

 private:
  KeyCodes() {
    for (const auto& pair : keycode_to_name_) {
      name_to_keycode_[pair.second] = pair.first;
    }
  }
  static std::shared_ptr<KeyCodes> instance_;

  const std::unordered_map<int, std::string> keycode_to_name_ = {
      {KEY_RESERVED, "KEY_RESERVED"},
      {KEY_ESC, "KEY_ESC"},
      {KEY_1, "KEY_1"},
      {KEY_2, "KEY_2"},
      {KEY_3, "KEY_3"},
      {KEY_4, "KEY_4"},
      {KEY_5, "KEY_5"},
      {KEY_6, "KEY_6"},
      {KEY_7, "KEY_7"},
      {KEY_8, "KEY_8"},
      {KEY_9, "KEY_9"},
      {KEY_0, "KEY_0"},
      {KEY_MINUS, "KEY_MINUS"},
      {KEY_EQUAL, "KEY_EQUAL"},
      {KEY_BACKSPACE, "KEY_BACKSPACE"},
      {KEY_TAB, "KEY_TAB"},
      {KEY_Q, "KEY_Q"},
      {KEY_W, "KEY_W"},
      {KEY_E, "KEY_E"},
      {KEY_R, "KEY_R"},
      {KEY_T, "KEY_T"},
      {KEY_Y, "KEY_Y"},
      {KEY_U, "KEY_U"},
      {KEY_I, "KEY_I"},
      {KEY_O, "KEY_O"},
      {KEY_P, "KEY_P"},
      {KEY_LEFTBRACE, "KEY_LEFTBRACE"},
      {KEY_RIGHTBRACE, "KEY_RIGHTBRACE"},
      {KEY_ENTER, "KEY_ENTER"},
      {KEY_LEFTCTRL, "KEY_LEFTCTRL"},
      {KEY_A, "KEY_A"},
      {KEY_S, "KEY_S"},
      {KEY_D, "KEY_D"},
      {KEY_F, "KEY_F"},
      {KEY_G, "KEY_G"},
      {KEY_H, "KEY_H"},
      {KEY_J, "KEY_J"},
      {KEY_K, "KEY_K"},
      {KEY_L, "KEY_L"},
      {KEY_SEMICOLON, "KEY_SEMICOLON"},
      {KEY_APOSTROPHE, "KEY_APOSTROPHE"},
      {KEY_GRAVE, "KEY_GRAVE"},
      {KEY_LEFTSHIFT, "KEY_LEFTSHIFT"},
      {KEY_BACKSLASH, "KEY_BACKSLASH"},
      {KEY_Z, "KEY_Z"},
      {KEY_X, "KEY_X"},
      {KEY_C, "KEY_C"},
      {KEY_V, "KEY_V"},
      {KEY_B, "KEY_B"},
      {KEY_N, "KEY_N"},
      {KEY_M, "KEY_M"},
      {KEY_COMMA, "KEY_COMMA"},
      {KEY_DOT, "KEY_DOT"},
      {KEY_SLASH, "KEY_SLASH"},
      {KEY_RIGHTSHIFT, "KEY_RIGHTSHIFT"},
      {KEY_KPASTERISK, "KEY_KPASTERISK"},
      {KEY_LEFTALT, "KEY_LEFTALT"},
      {KEY_SPACE, "KEY_SPACE"},
      {KEY_CAPSLOCK, "KEY_CAPSLOCK"},
      {KEY_F1, "KEY_F1"},
      {KEY_F2, "KEY_F2"},
      {KEY_F3, "KEY_F3"},
      {KEY_F4, "KEY_F4"},
      {KEY_F5, "KEY_F5"},
      {KEY_F6, "KEY_F6"},
      {KEY_F7, "KEY_F7"},
      {KEY_F8, "KEY_F8"},
      {KEY_F9, "KEY_F9"},
      {KEY_F10, "KEY_F10"},
      {KEY_NUMLOCK, "KEY_NUMLOCK"},
      {KEY_SCROLLLOCK, "KEY_SCROLLLOCK"},
      {KEY_KP7, "KEY_KP7"},
      {KEY_KP8, "KEY_KP8"},
      {KEY_KP9, "KEY_KP9"},
      {KEY_KPMINUS, "KEY_KPMINUS"},
      {KEY_KP4, "KEY_KP4"},
      {KEY_KP5, "KEY_KP5"},
      {KEY_KP6, "KEY_KP6"},
      {KEY_KPPLUS, "KEY_KPPLUS"},
      {KEY_KP1, "KEY_KP1"},
      {KEY_KP2, "KEY_KP2"},
      {KEY_KP3, "KEY_KP3"},
      {KEY_KP0, "KEY_KP0"},
      {KEY_KPDOT, "KEY_KPDOT"},

      {KEY_ZENKAKUHANKAKU, "KEY_ZENKAKUHANKAKU"},
      {KEY_102ND, "KEY_102ND"},
      {KEY_F11, "KEY_F11"},
      {KEY_F12, "KEY_F12"},
      {KEY_RO, "KEY_RO"},
      {KEY_KATAKANA, "KEY_KATAKANA"},
      {KEY_HIRAGANA, "KEY_HIRAGANA"},
      {KEY_HENKAN, "KEY_HENKAN"},
      {KEY_KATAKANAHIRAGANA, "KEY_KATAKANAHIRAGANA"},
      {KEY_MUHENKAN, "KEY_MUHENKAN"},
      {KEY_KPJPCOMMA, "KEY_KPJPCOMMA"},
      {KEY_KPENTER, "KEY_KPENTER"},
      {KEY_RIGHTCTRL, "KEY_RIGHTCTRL"},
      {KEY_KPSLASH, "KEY_KPSLASH"},
      {KEY_SYSRQ, "KEY_SYSRQ"},
      {KEY_RIGHTALT, "KEY_RIGHTALT"},
      {KEY_LINEFEED, "KEY_LINEFEED"},
      {KEY_HOME, "KEY_HOME"},
      {KEY_UP, "KEY_UP"},
      {KEY_PAGEUP, "KEY_PAGEUP"},
      {KEY_LEFT, "KEY_LEFT"},
      {KEY_RIGHT, "KEY_RIGHT"},
      {KEY_END, "KEY_END"},
      {KEY_DOWN, "KEY_DOWN"},
      {KEY_PAGEDOWN, "KEY_PAGEDOWN"},
      {KEY_INSERT, "KEY_INSERT"},
      {KEY_DELETE, "KEY_DELETE"},
      {KEY_MACRO, "KEY_MACRO"},
      {KEY_MUTE, "KEY_MUTE"},
      {KEY_VOLUMEDOWN, "KEY_VOLUMEDOWN"},
      {KEY_VOLUMEUP, "KEY_VOLUMEUP"},
      {KEY_POWER, "KEY_POWER"},
      {KEY_KPEQUAL, "KEY_KPEQUAL"},
      {KEY_KPPLUSMINUS, "KEY_KPPLUSMINUS"},
      {KEY_PAUSE, "KEY_PAUSE"},
      {KEY_SCALE, "KEY_SCALE"},

      {KEY_KPCOMMA, "KEY_KPCOMMA"},
      {KEY_HANGEUL, "KEY_HANGEUL"},
      {KEY_HANJA, "KEY_HANJA"},
      {KEY_YEN, "KEY_YEN"},
      {KEY_LEFTMETA, "KEY_LEFTMETA"},
      {KEY_RIGHTMETA, "KEY_RIGHTMETA"},
      {KEY_COMPOSE, "KEY_COMPOSE"},

      {KEY_STOP, "KEY_STOP"},
      {KEY_AGAIN, "KEY_AGAIN"},
      {KEY_PROPS, "KEY_PROPS"},
      {KEY_UNDO, "KEY_UNDO"},
      {KEY_FRONT, "KEY_FRONT"},
      {KEY_COPY, "KEY_COPY"},
      {KEY_OPEN, "KEY_OPEN"},
      {KEY_PASTE, "KEY_PASTE"},
      {KEY_FIND, "KEY_FIND"},
      {KEY_CUT, "KEY_CUT"},
      {KEY_HELP, "KEY_HELP"},
      {KEY_MENU, "KEY_MENU"},
      {KEY_CALC, "KEY_CALC"},
      {KEY_SETUP, "KEY_SETUP"},
      {KEY_SLEEP, "KEY_SLEEP"},
      {KEY_WAKEUP, "KEY_WAKEUP"},
      {KEY_FILE, "KEY_FILE"},
      {KEY_SENDFILE, "KEY_SENDFILE"},
      {KEY_DELETEFILE, "KEY_DELETEFILE"},
      {KEY_XFER, "KEY_XFER"},
      {KEY_PROG1, "KEY_PROG1"},
      {KEY_PROG2, "KEY_PROG2"},
      {KEY_WWW, "KEY_WWW"},
      {KEY_MSDOS, "KEY_MSDOS"},
      {KEY_COFFEE, "KEY_COFFEE"},
      {KEY_ROTATE_DISPLAY, "KEY_ROTATE_DISPLAY"},
      {KEY_CYCLEWINDOWS, "KEY_CYCLEWINDOWS"},
      {KEY_MAIL, "KEY_MAIL"},
      {KEY_BOOKMARKS, "KEY_BOOKMARKS"},
      {KEY_COMPUTER, "KEY_COMPUTER"},
      {KEY_BACK, "KEY_BACK"},
      {KEY_FORWARD, "KEY_FORWARD"},
      {KEY_CLOSECD, "KEY_CLOSECD"},
      {KEY_EJECTCD, "KEY_EJECTCD"},
      {KEY_EJECTCLOSECD, "KEY_EJECTCLOSECD"},
      {KEY_NEXTSONG, "KEY_NEXTSONG"},
      {KEY_PLAYPAUSE, "KEY_PLAYPAUSE"},
      {KEY_PREVIOUSSONG, "KEY_PREVIOUSSONG"},
      {KEY_STOPCD, "KEY_STOPCD"},
      {KEY_RECORD, "KEY_RECORD"},
      {KEY_REWIND, "KEY_REWIND"},
      {KEY_PHONE, "KEY_PHONE"},
      {KEY_ISO, "KEY_ISO"},
      {KEY_CONFIG, "KEY_CONFIG"},
      {KEY_HOMEPAGE, "KEY_HOMEPAGE"},
      {KEY_REFRESH, "KEY_REFRESH"},
      {KEY_EXIT, "KEY_EXIT"},
      {KEY_MOVE, "KEY_MOVE"},
      {KEY_EDIT, "KEY_EDIT"},
      {KEY_SCROLLUP, "KEY_SCROLLUP"},
      {KEY_SCROLLDOWN, "KEY_SCROLLDOWN"},
      {KEY_KPLEFTPAREN, "KEY_KPLEFTPAREN"},
      {KEY_KPRIGHTPAREN, "KEY_KPRIGHTPAREN"},
      {KEY_NEW, "KEY_NEW"},
      {KEY_REDO, "KEY_REDO"},

      {KEY_F13, "KEY_F13"},
      {KEY_F14, "KEY_F14"},
      {KEY_F15, "KEY_F15"},
      {KEY_F16, "KEY_F16"},
      {KEY_F17, "KEY_F17"},
      {KEY_F18, "KEY_F18"},
      {KEY_F19, "KEY_F19"},
      {KEY_F20, "KEY_F20"},
      {KEY_F21, "KEY_F21"},
      {KEY_F22, "KEY_F22"},
      {KEY_F23, "KEY_F23"},
      {KEY_F24, "KEY_F24"},

      {KEY_PLAYCD, "KEY_PLAYCD"},
      {KEY_PAUSECD, "KEY_PAUSECD"},
      {KEY_PROG3, "KEY_PROG3"},
      {KEY_PROG4, "KEY_PROG4"},
      {KEY_ALL_APPLICATIONS, "KEY_ALL_APPLICATIONS"},
      {KEY_SUSPEND, "KEY_SUSPEND"},
      {KEY_CLOSE, "KEY_CLOSE"},
      {KEY_PLAY, "KEY_PLAY"},
      {KEY_FASTFORWARD, "KEY_FASTFORWARD"},
      {KEY_BASSBOOST, "KEY_BASSBOOST"},
      {KEY_PRINT, "KEY_PRINT"},
      {KEY_HP, "KEY_HP"},
      {KEY_CAMERA, "KEY_CAMERA"},
      {KEY_SOUND, "KEY_SOUND"},
      {KEY_QUESTION, "KEY_QUESTION"},
      {KEY_EMAIL, "KEY_EMAIL"},
      {KEY_CHAT, "KEY_CHAT"},
      {KEY_SEARCH, "KEY_SEARCH"},
      {KEY_CONNECT, "KEY_CONNECT"},
      {KEY_FINANCE, "KEY_FINANCE"},
      {KEY_SPORT, "KEY_SPORT"},
      {KEY_SHOP, "KEY_SHOP"},
      {KEY_ALTERASE, "KEY_ALTERASE"},
      {KEY_CANCEL, "KEY_CANCEL"},
      {KEY_BRIGHTNESSDOWN, "KEY_BRIGHTNESSDOWN"},
      {KEY_BRIGHTNESSUP, "KEY_BRIGHTNESSUP"},
      {KEY_MEDIA, "KEY_MEDIA"},

      {KEY_SWITCHVIDEOMODE, "KEY_SWITCHVIDEOMODE"},
      {KEY_KBDILLUMTOGGLE, "KEY_KBDILLUMTOGGLE"},
      {KEY_KBDILLUMDOWN, "KEY_KBDILLUMDOWN"},
      {KEY_KBDILLUMUP, "KEY_KBDILLUMUP"},

      {KEY_SEND, "KEY_SEND"},
      {KEY_REPLY, "KEY_REPLY"},
      {KEY_FORWARDMAIL, "KEY_FORWARDMAIL"},
      {KEY_SAVE, "KEY_SAVE"},
      {KEY_DOCUMENTS, "KEY_DOCUMENTS"},

      {KEY_BATTERY, "KEY_BATTERY"},

      {KEY_BLUETOOTH, "KEY_BLUETOOTH"},
      {KEY_WLAN, "KEY_WLAN"},
      {KEY_UWB, "KEY_UWB"},

      {KEY_UNKNOWN, "KEY_UNKNOWN"},

      {KEY_VIDEO_NEXT, "KEY_VIDEO_NEXT"},
      {KEY_VIDEO_PREV, "KEY_VIDEO_PREV"},
      {KEY_BRIGHTNESS_CYCLE, "KEY_BRIGHTNESS_CYCLE"},
      {KEY_BRIGHTNESS_AUTO, "KEY_BRIGHTNESS_AUTO"},
      {KEY_DISPLAY_OFF, "KEY_DISPLAY_OFF"},

      {KEY_WWAN, "KEY_WWAN"},
      {KEY_RFKILL, "KEY_RFKILL"},

      {KEY_MICMUTE, "KEY_MICMUTE"},
  };

  std::unordered_map<std::string, int> name_to_keycode_;
};

// For convenience.

std::string keyCodeToName(int key_code);

std::optional<int> nameToKeyCode(std::string name);

#endif  // __KEYCODE_LOOKUP_H
