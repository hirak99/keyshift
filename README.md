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
- E.g. NUM- to Volume Up button.
- E.g. CAPSLOCK when held, activates certain mapping layer.
- E.g. Shift when held, Esc acts as Backtick (resulting in Shift+Backtick = ~).
- E.g. Ctrl when held, 1 leads to F1 (_not_ Ctrl+F1), but every other key work as Ctrl+key.
- (TBD) E.g. Del when held, if any other key pressed it uses some specified mapping. If nothing else pressed, output Del.
- (TBD) Macros: E.g. (SHIFT + CAPSLOCK) -> "H E L L O".
- (TBD) Pause within macros (will require threading).

Other considerations -
- (TBD) Make it configurable if a layer passes through or blocks all keys by default?

Example json mapping -

Here -
- 'KEY' = tap a key, equivalent to ['^KEY', '~KEY']
- mappings are {event: outcome(s)}

```json
{
    "mapppings": {
        "[default]": {
            ":passthru": "_ALL",

            // As long as CAPSLOCK is held, use the func_layer.
            "^CAPSLOCK": ":activate [func_layer]",

            // When pressed, hold the key and activate its layer.
            "^RIGHT_CTRL": [
                "^RIGHT_CTRL",
                ":activate [rctrl_func_layer]",
            ],
            "^LEFT_SHIFT": [
                "^LEFT_SHIFT",
                ":activate [lshift_layer]",
            ],

            // Snap tap.
            "^D": ["~A", "^D"],
            "^A": ["~D", "^A"],
        },
        "[lshift_layer]": {
            "ESC": "`",  // Since LEFT_SHIFT will be held, this will produce ~.
            ":passthru": "_ALL",
        },
        "[func_layer]": {
            "1": "F1",
            "2": "F2",
            ":passthru": [],
        },
        "[rctrl_func_layer]": {
            "_pre": "~RIGHT_CTRL",
            "1": "F1",
            "2": "F2",
            ":passthru": [],
        },
    }
}
```

More advanced configurations -
- If Esc is held for 500ms, do something, else do something else.
- If Del is held and another key is pressed, act as a layer. But if Del is
  released, just act as tap.

Note that in these scenarios, it is impossible to process the key immediately,
and keys will be emitted only when the outcome is fully determined.

```json
{
    "mapppings": {
        "[default]": {
            // If ESC is held for more than 500ms, it acts as BACKTICK.
            // But if it is released before, it acts as ESC.
            "ESC": [":if_held_for 500ms", "BACKTICK", "ESC"],

            // When DELETE is held and another key sis pressed, use del_layer.
            // If DELETE is release before other key is pressed, just tap DELETE.
            "DELETE": [":if_another_key", ":activate [del_layer]", "DELETE"],
        },
        "[del_layer]": {
            "END": "VOLUME_UP",
        },
    }
}
```

Internally, the DELETE layer may be implemented like this -
```json
{
    "^DELETE": [":activate del_layer"],
    "[del_layer]": {
        "~DELETE": ["DELETE", ":deactivate_layer"],
        "END": ["VOLUME_UP", ":deactivate_layer"],
    }
}
```

## Alternative Super Simple Specs?

```
CAPSLOCK + 1 = F1
CAPSLOCK + 2 = F2

// The line below enables the button explicitly, even if it also activates other keys.
^RIGHT_CTRL = ^RIGHT_CTRL
RIGHT_CTRL + 1 = ~RIGHT_CTRL F1
RIGHT_CTRL + 2 = ~RIGHT_CTRL F2
RIGHT_CTRL + * = *  // Any key not defined is passed thru.

// Allows Shift+Esc = ~.
^SHIFT = ^SHIFT
SHIFT + ESC = BACKTICK
SHIFT + * = *

// Snap tap.
^D = ~A ^D
^A = ~D ^A

ESC + 500ms = BACKTICK

DEL + END = VOLUME_UP
DEL + nothing = DEL  # Do a DEL if nothing else happened.
```

Note: If multiple layers are active, all active layers will be iterated in
reverse order, as long as the key is passed through, until a mapping is found.

# Tutorial

## Experimenting

Locate your keyboard in "/dev/input/by-id".

Then try this -

```sh
keyboard_remap "/dev/input/by-id/...-kbd" --config-string "A=B;B=A"
```

Verify that this swaps the keys A and B as long as it is running. You can exit out of it with Ctrl+C.


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
  - `KEY1 + * = *` - Allow all keys not explicitly remapped under KEY1 to pass thru as is.
  - `KEY1 + nothing = [TOKEN ...]` - Specifies what should happen if nothing inside the layer is activated. E.g. `DELETE + 1 = F1; DELETE + nothing = DELETE` will ensure DELETE acts as itself unless 1 is pressed within it.

## Examples

- Swap keys

```
// Swap A and B keys.
A = B
B = A
```
