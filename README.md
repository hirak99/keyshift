# Keyboard Remapper

# Philosophy

## Simplicity and Performance

An important goal is to achieve both high performance and simplicity in our code. This encompasses optimizing for speed, minimizing memory usage, and efficiently managing threads.

The choice of programming language is crucial to this objective. Although no language can guarantee top-tier performance on its own, it significantly influences the maximum performance potential we can reach. In balancing performance and simplicity, we have selected C++ as our language of choice.

## Linux First

We make this work only on Linux. This helps with our first goal of simplicity.

That said, a part of the code may be re-usable for other operating systems. But other operating systems will not be in our scope for a long time.

## Maintainable

As part of maintainability, we will keep the code readable. We will for now follow Google style guide for c++.

# Building

## Dependencies

Following c++ libraries are needed -
- Boost
- Catch2 (for tests only)

## Build

Run the following commands to build and test -

```sh
cd src
mkdir -p build
cd build
cmake ..
make -j
ctest
```

# Remapping Needs

## Use Cases
- E.g. NUM-MINUS to Volume Up button.
- E.g. CAPSLOCK when held, activates certain mapping layer.
- E.g. Shift when held, Esc acts as Backtick (resulting in Shift+Backtick = ~).
- E.g. Ctrl when held, 1 leads to F1 (_not_ Ctrl+F1), but every other key work as Ctrl+key.
- E.g. Del when held, if any other key pressed it uses some specified mapping. If nothing else pressed, output Del.
- Macros: E.g. (SHIFT + CAPSLOCK) -> "H E L L O".
- (TBD) Pause within macros (will require threading).
- (TBD) Hold ESC for 500ms to act as backtick.

## Example configuration

```
CAPSLOCK + 1 = F1
CAPSLOCK + 2 = F2

// The line below enables the button explicitly, even if it also activates other keys.
^RIGHTCTRL = ^RIGHTCTRL
RIGHTCTRL + 1 = ~RIGHTCTRL F1
RIGHTCTRL + 2 = ~RIGHTCTRL F2
RIGHTCTRL + * = *  // Any key not defined is passed thru.

// Allows Shift+Esc = ~.
^LEFTSHIFT = ^LEFTSHIFT
LEFTSHIFT + ESC = GRAVE  // GRAVE is backtick/tilde key.
LEFTSHIFT + * = *

// Snap tap.
^D = ~A ^D
^A = ~D ^A

DELETE + END = VOLUMEUP
DELETE + nothing = DELETE  // Do a DELETE if nothing else happened.
```

# Tutorial

## Experimenting

Locate your keyboard in "/dev/input/by-id".

Then try this -

```sh
keyboard_remap "/dev/input/by-id/...-kbd" --config-string "A=B;B=A"
```

Verify that this swaps the keys A and B as long as it is running. You can exit out of it with Ctrl+C.

## How to find keycodes

Run -
```sh
sudo evtest /dev/input/by-id/...-kbd
```

Then press any key that you want to know the id of.

## Examples

- Remap keys
```
// Remap menu key (KEY_PASTE) into Right Win key.
PASTE = RIGHTMETA

// Swap A and B keys.
A = B
B = A
```

- Make CAPSLOCK to behave as Fn key

```
CAPSLOCK + 1 = F1
CAPSLOCK + 2 = F2
// ...
CAPSLOCK + MINUS = F11
CAPSLOCK + EQUAL = F12

// The capslock key is now will no longer function as itself.
// We can however use an alternative way to trigger it.
CAPSLOCK + TAB = CAPSLOCK

// Capslock passthrus.
CAPSLOCK + LEFTALT = *  // Since we want CAPSLOCK+LEFTALT+4 = Alt+F4.
CAPSLOCK + RIGHTALT = *
CAPSLOCK + LEFTCTRL = *
```

- Make Left Shift + Esc = ~, leave shift as is otherwise

```
^LEFTSHIFT = ^LEFTSHIFT  // Holding shift will actually press shift.
LEFTSHIFT + ESC = GRAVE  // And reassign other keys. GRAVE is the `/~ key.
LEFTSHIFT + * = *        // All other keys while holding shift are unaltered.
```

- Make Right Ctrl behave as is, except Ctrl+0 as F10 _without_ Ctrl.
```
^RIGHTCTRL = ^RIGHTCTRL
RIGHTCTRL + 0 = ~RIGHTCTRL F10  // Leave the key and press F10.
RIGHTCTRL + * = *               // Let anything else pass thru.
```

- DEL itself is DEL, but DEL+END is Volume Up

```
DELETE + END =  VOLUMEUP
// ... more DELETE + combos can be assigned here.

// Acts as DELETE if nothing is pressed before DELETE is released.
DELETE + nothing = DELETE
```

- Snap Tap
  - Snaptap is a feature where pressing a key immediately deactivates some other key.
  - WARNING: For Counter Strike, this was used with A and D keys, as in the example below. This is now banned in Counter Strike 2 for official servers.

```
// Pressing A will first release D (if D is pressed).
^A = ~D ^A
// And vice versa.
^D = ~A ^D
```

## Statement Reference

- Available keycodes
  - You can use any keycode [defined here](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h). You can omit the "KEY_" prefix.
  - As special tokens, you can use `*` and `nothing`. See descriptions below.

- Basic remapping
  - `^KEY1 = [TOKEN ...]` - The `^` indicates press of a key. On KEY1 press, tokens on the right will be performed.
  - `~KEY1 = [TOKEN ...]` - The `~` indicates release of a key. On KEY1 release, tokens on the right will be performed.
  - `KEY1 = [TOKEN ...] FINAL_TOKEN` - Equivalent to `^KEY1 = [TOKEN ...] ^FINAL_TOKEN`, `~KEY1 = ~FINAL_TOKEN`. Example: `A = B` will make the A key act exactly like B.
  - `KEY1 = nothing` - Blocks the key. E.g. `DELETE = nothing`.

- Layering
  - `KEY1 + KEY2 = [TOKEN ...]` - Only if KEY1 is held, KEY2 will activate the tokens.
    - Note that by default the layer-activation key, i.e. KEY1 in this example, will now be suppressed. If you need to also register the activation key, you must specify that explicitly `^KEY1 = ^KEY1`.
  - `KEY1 + KEY2 = *` - Shorthand for `KEY1 + KEY2 = KEY2`. Allows the key KEY2 to be passed thru in this layer unaltered.
  - `KEY1 + * = *` - Allow all keys not explicitly remapped under KEY1 to pass thru as is.
  - `KEY1 + nothing = [TOKEN ...]` - Specifies what should happen if nothing inside the layer is activated. E.g. `DELETE + 1 = F1; DELETE + nothing = DELETE` will ensure DELETE acts as itself unless 1 is pressed within it.
