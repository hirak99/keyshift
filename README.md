# Keyboard Remapper

A fast, simple and powerful keyboard remapping and layering software.

This emulates Fn key of your laptop on any keyboard, and a lot more.

# Inspiration: Kmonad

Kmonad is excellent. It is unbeatable in functionality. If you are happy with kmonad, by all means use it!

This was inspired by kmonad, as a performant, easy to use alternative.

I needed something for gaming - and it wanted it to be performant and powerful.

For these reasons I decided to experiment, and it grew into a new project. Along with performance, this now also offers a simple and easy to use config language which is different from kmonad.

Keeping these in mind, table below lists and compares a few properties.

| .                               | Kmonad              | KeyShift                                         |
| ------------------------------- | ------------------- | ------------------------------------------------ |
| Threads ¹                       | 34                  | 1                                                |
| RAM ¹                           | 52M (RES)           | < 10M (RES)                                       |
| Latency overhead ²              | Unknown             | < 0.02 ms                                        |
| Language                        | Haskell             | C++ 23                                           |
| Functionality: Layering support | ✓                   | ✓ E.g. `CAPSLOCK+1=F1`                           |
| Functionality: Dual function    | ✓                   | ✓ E.g. `CAPSLOCK+1=F1;CAPSLOCK+nothing=CAPSLOCK` |
| Functionality: Snap tap         | ✓                   | ✓ E.g. `^A=~D ^A;^D=~A ^D`                       |
| Functionality: Key timeouts     | ✓                   | ✗                                                |
| OS Support                      | Linux, Windows, Mac | Linux only                                       |

Note 1: Threads & RAM usage were measured on equivalent key-mapping configuration, on same hardware.

Note 2: The profiling of KeyShift is based on average time spent in `Process(int, int)` on the commit 62f3421, based on an artificial load (see
profile.cpp), on an i9-9900k.

# Philosophy

## Performance

An important goal is to achieve high performance. This encompasses optimizing for speed, minimizing memory usage, and efficiently managing threads.

The choice of programming language is crucial to this objective. Although no language can guarantee top-tier performance on its own, it significantly influences the maximum performance potential we can reach. In balancing performance and simplicity, we have selected C++ as our language of choice.

Some amount of latency, e.g. communicating with the hardware, will be unavoidable. Our goal will be to keep any additional latency well below a reasonable budget.

Latency goals -
- Measuring: Knowing the latency will help us track and reduce it.
- Budget: IMHO, for competitive gaming <2ms is an acceptable tolerance. We will attempt to keep the additional overhead below that.


## Simplicity & Power

The program should be powerful to express all basic needs from a keyboard remapping software. Please see the "Use Cases" section below.

The configuration system should be as simple and intuitive as it can be, but no simpler.

## Linux First

The current scope of the project is restricted only to Linux.

That said, a large part of the codebase is re-usable OS independent, meaning it may come to other OS eventually.

# Bugs / Feature Requests

If you encounter any issues or find that something isn't working as expected, please open an issue here.

If there is a feature that you consider important and it's not currently available, feel free to open an issue to request it.

While not all feature requests can be implemented, I will review each request to determine if it aligns with the project’s scope and priorities. Thank you for your input!

# Building

## Dependencies

Following c++ libraries are needed -
- Boost
- Catch2 (for tests only)

## Build

Run the following commands to build and test -

```sh
mkdir -p build
cd build
cmake ../src
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

We intend to keep the configuration language simple and intuitive. Below is an example. See also the examples/ directory.

```
CAPSLOCK + 1 = F1
CAPSLOCK + 2 = F2

CAPSLOCK + LEFTSHIFT = CAPSLOCK

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

# Tutorial

## Experimenting

Locate your keyboard in "/dev/input/by-id".

Then try this -

```sh
keyboard_remap "/dev/input/by-id/...-kbd" --config "A=B;B=A"
```

Verify that this swaps the keys A and B as long as it is running. You can exit out of it with Ctrl+C.

You can also load the configuration from a file instead with `--config-file /path/to/config.keyshift`.

Once you are happy with a configuration, you can add it to your startup. Or, you can add it to udev so that it activates whenever a particular keyboard is plugged in.

## How to find keycodes

Run -
```sh
sudo evtest /dev/input/by-id/...-kbd
```

Then press any key that you want to know the id of.

All keycodes [defined in linux/input-event-codes.h](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h) are supported. You can also look for a keycode there to map to, which is not available on your keyboard.

## Example Tasks

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
CAPSLOCK + LEFTSHIFT = CAPSLOCK

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

## Safety

If you lock yourself out due to a bad configuration, don't fret. There is a kill combo that deactivates keyshift. Type these keys in order while keyshift is running -

`KEYSHIFTRESERVEDCMDKILL`

Don't worry if you modified those keys with a configuration, the combo acts on actual keys.

You will see a std::runtime_error, which is normal in this case and confirms deactivation.
