# Remapping Needs

This is a keyboard layering software.

Keyboard layering allows you to create multiple sets of key functions that can be accessed through different states or modes of the keyboard. This means that pressing a specific key or combination can switch to a new layer, enabling different key mappings and shortcuts without the need for additional physical keys.

In other words, this enables you to (1) remap keys, and (2) to define `Fn`-like keys that change how other keys behave.

## Use Cases

As a design principle, we start with our desired outcomes first. These help to work backward from there and build the configuration language elements and a system to implement them.

Below are the classes of use cases considered.

| Class            | Example Usage                                                                                                        | Status      |
| ---------------- | -------------------------------------------------------------------------------------------------------------------- | ----------- |
| Simple Remap     | E.g. NUM-STAR/MINUS to Volume Up/Down buttons.                                                                       | ✓           |
| Layers           | E.g. CAPSLOCK held converts numbers to function keys.                                                                | ✓           |
| Around           | E.g. Shift operates as is, but also when held, Esc acts as Backtick (resulting in Shift+Backtick = ~).               | ✓           |
| Multi Functional | E.g. Ctrl when held acts as Ctrl. But when 1 is pressed, F1 is emitted without Ctrl - and same for other numbers.    | ✓           |
| Normal or Layer  | E.g. if Del is tapped, it acts as Del. But when Del when held and another key is pressed, it acts as a layering key. | ✓           |
| Macros           | E.g. (SHIFT + CAPSLOCK) -> "H E L L O".                                                                              | ✓           |
| Pause in Macro   | E.g. "H 50ms E 50ms L 50ms L 50ms O".                                                                                | ✓           |
| Timed Function   | E.g. hold ESC for 500ms to act as backtick.                                                                          | Not planned |

## Example configuration

Note: Supported keycodes can be found in [/usr/include/linux/input-event-codes.h](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h)
```
// == Layers ==
CAPSLOCK + 1 = F1
CAPSLOCK + 2 = F2
// ...

CAPSLOCK + LEFTSHIFT = CAPSLOCK  // Make it also possible to trigger CAPSLOCK itself.

// == Multi Functional ==
// The line below enables the button explicitly, even if it also activates other keys.
^RIGHTCTRL = ^RIGHTCTRL
RIGHTCTRL + 1 = ~RIGHTCTRL F1
RIGHTCTRL + 2 = ~RIGHTCTRL F2
RIGHTCTRL + * = *  // Any key not defined is passed thru.

// Allows Shift+Esc = ~.
^LEFTSHIFT = ^LEFTSHIFT
LEFTSHIFT + ESC = GRAVE  // GRAVE is backtick/tilde key.
LEFTSHIFT + * = *

// Snap tap - pressing A will release D first, and vice versa.
^D = ~A ^D
^A = ~D ^A

DELETE + END = VOLUMEUP
DELETE + nothing = DELETE  // Do a DELETE if no other key is pressed.
```

Internally it creates layers when it parses the config. The parsed results can be viewed with `--dump` flag, e.g. `keyshift --dump --config='CAPSLOCK+1=F1'`.
