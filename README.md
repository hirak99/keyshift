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
