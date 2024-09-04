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
            "^CAPSLOCK": ":activate func_keys",
            "~CAPSLOCK": ":deactivate func_keys",
            "^RIGHT_CTRL": [
                "^RIGHT_CTRL",
                [":if :next in :[func_keys]", ["~RIGHT_CTRL", ":[func_keys](:next)"]]
            ],
            "^LEFT_SHIFT": [
                "^LEFT_SHIFT",
                [":if :next is ESC", "BACKTICK"]
            ],
            # Snap tap.
            "^D": ["~A", "^D"],
            "^A": ["~D", "^A"],
        },
        "[func_keys]": {"1": "F1", "2": "F2"}
    }
}
```