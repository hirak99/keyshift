# Keyboard Remapper

A fast, simple and powerful keyboard remapping and layering software.

This emulates Fn key of your laptop on any keyboard, and a lot more.

# Quick Start

## Installing

### Archlinux
[Available in AUR](https://aur.archlinux.org/packages/keyshift), you can use your favorite AUR manager to install -

`yay -S keyshift`

### Others
```sh
git clone --recursive https://github.com/hirak99/keyshift
cd keyshift

# Note: You may need to install "boost" for your distribution if the build fails.
./build.sh

# Use it locally.
./build/keyshift --help
# Or copy the binary to your system.
install -Dm 755 ./build/keyshift /usr/local/bin/keyshift
```

## Trying

Locate your keyboard in "/dev/input/by-id".

Then run this -

```sh
sudo keyshift "/dev/input/by-id/...-kbd" --config "A=B;B=A"
```

This will swap the A and B keys.

You can build more powerful config and store it in a file. When fully
configured, this is intended to run in the background covering the session.

For now you can exit pressing Ctrl+C.

# Goals

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

# Inspiration: KMonad

KMonad is a fantastic tool with great functionality, and if it works well for
you, I encourage you to continue using it!

This new project was inspired by a desire to create an alternative that focuses
on performance and simplicity.

My aim was to develop something specifically for gaming - efficient and
powerful.

As I experimented, this idea evolved into a new project. In addition to enhanced
performance, it now features a user-friendly configuration language that offers
a different approach from KMonad. For these reasons I decided to experiment, and
it grew into a new project. Along with performance, this now also offers a
simple and easy to use config language which is different from kmonad.

## Comparison with KMonad

Both KMonad and KeyShift offer unique advantages tailored to different user
needs. While KMonad excels in various areas, KeyShift aims to provide a more
streamlined experience for gaming. Below is a comparison of some key properties:


|                                 | KMonad              | KeyShift                                         |
| ------------------------------- | ------------------- | ------------------------------------------------ |
| Functionality: Layering support | ✓                   | ✓ E.g. `CAPSLOCK+1=F1`                           |
| Functionality: Dual function    | ✓                   | ✓ E.g. `CAPSLOCK+1=F1;CAPSLOCK+nothing=CAPSLOCK` |
| Functionality: Snap tap         | ✓                   | ✓ E.g. `^A=~D ^A;^D=~A ^D`                       |
| Functionality: Key timeouts     | ✓                   | ✗                                                |
| Threads ¹                       | 34                  | 1                                                |
| RAM ¹                           | 52M (RES)           | < 10M (RES)                                      |
| Latency overhead ²              | Unknown             | < 0.02 ms                                        |
| Language                        | Haskell             | C++ 23                                           |
| OS Support                      | Linux, Windows, Mac | Linux only                                       |

Note 1: Threads & RAM usage were measured on equivalent key-mapping configuration, on same hardware.

Note 2: The profiling of KeyShift is based on average time spent in
`Process(int, int)` on the commit 62f3421, based on an artificial load (see
profile.cpp), on an i9-9900k.

# Bugs / Feature Requests

If you encounter any issues or find that something isn't working as expected, please open an issue here.

If there is a feature that you consider important and it's not currently available, feel free to open an issue to request it.

While not all feature requests can be implemented, I will review each request to determine if it aligns with the project’s scope and priorities. Thank you for your input!

# Building

## Dependencies

Following c++ libraries are needed -
- Boost
- Catch2 (only for tests, not required for building the production binary)

## Commands to Build

See `build.sh`.

# Remapping Needs

## Use Cases

As a design principle, we start with our desired outcomes first. These help to work backward from there and build the configuration language elements and a system to implement them.

Below are the classes of use cases considered.

| Class            | Example Usage                                                                                                                                      | Status      |
| ---------------- | -------------------------------------------------------------------------------------------------------------------------------------------------- | ----------- |
| Simple Remap     | E.g. NUM-STAR/MINUS to Volume Up/Down buttons.                                                                                                     | ✓           |
| Layers           | E.g. CAPSLOCK held converts numbers to function keys.                                                                                              | ✓           |
| Around           | E.g. Shift operates as is, but also when held, Esc acts as Backtick (resulting in Shift+Backtick = ~).                                             | ✓           |
| Multi Functional | E.g. Ctrl when held acts as Ctrl. But when 1 is pressed, F1 is emitted without Ctrl - and same for other numbers.                                  | ✓           |
| Normal or Layer  | E.g. if Del is tapped, it acts as Del. But when Del when held and another key is pressed, it acts as a layering key and Del itself is not emitted. | ✓           |
| Macros           | E.g. (SHIFT + CAPSLOCK) -> "H E L L O".                                                                                                            | ✓           |
| Pause in Macro   | E.g. "H 50ms E 50ms L 50ms L 50ms O".                                                                                                              | Not planned |
| Timed Function   | E.g. hold ESC for 500ms to act as backtick.                                                                                                        | Not planned |

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

# Tutorial

## Experimenting

Locate your keyboard in "/dev/input/by-id".

Then try this -

```sh
sudo keyshift "/dev/input/by-id/...-kbd" --config "A=B;B=A"
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

All keycodes [/usr/include/linux/input-event-codes.h](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h) are supported. You can also look for a keycode there to map to, which is not available on your keyboard.

## Example Tasks

- **Remap keys**
```
// Remap menu key (KEY_PASTE) into Right Win key.
PASTE = RIGHTMETA

// Swap A and B keys.
A = B
B = A
```

- **Create a macro**

```
LEFTCTRL + H = H E L L O

// You can also insert pauses, such as -
LEFTCTRL + H = H 50ms E 50ms L 50ms L 50ms O
```

- **Make CAPSLOCK behave as Fn key**

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

- **Make Left Shift + Esc = ~, and leave shift as is otherwise.**

```
^LEFTSHIFT = ^LEFTSHIFT  // Holding shift will actually press shift.
LEFTSHIFT + ESC = GRAVE  // And reassign other keys. GRAVE is the `/~ key.
LEFTSHIFT + * = *        // All other keys while holding shift are unaltered.
```

- **Make Right Ctrl behave as is, except Ctrl+0 as F10 _without_ Ctrl.**
```
^RIGHTCTRL = ^RIGHTCTRL
RIGHTCTRL + 0 = ~RIGHTCTRL F10  // Leave the key and press F10.
RIGHTCTRL + * = *               // Let anything else pass thru.
```

- **Leave DEL by itself is DEL, but make DEL+END trigger Volume Up.**

```
DELETE + END =  VOLUMEUP
// ... more DELETE + combos can be assigned here.

// Acts as DELETE if nothing is pressed before DELETE is released.
DELETE + nothing = DELETE
```

- **Snap Tap**
  - Snaptap is a feature where pressing a key immediately deactivates some other key.
  - WARNING: For Counter Strike, this was used with A and D keys, as in the example below. This is now banned in Counter Strike 2 for official servers.

```
// Pressing A will first release D (if D is pressed).
^A = ~D ^A
// And vice versa.
^D = ~A ^D
```

## Comments

You can use either `#` or `//` for line comments.

Block comments are not supported.

Example -

```
// This is a comment.

# This is also a comment.

A = B  // Make A act as the B key.
```

## Statement Reference

- Available keycodes
  - You can use any keycode [/usr/include/linux/input-event-codes.h](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h). You can omit the "KEY_" prefix.
  - As special tokens, you can use `*` and `nothing`. See descriptions below.

- Basic remapping
  - `^KEY1 = [TOKEN ...]` - The `^` indicates press of a key. On KEY1 press, tokens on the right will be performed.
  - `~KEY1 = [TOKEN ...]` - The `~` indicates release of a key. On KEY1 release, tokens on the right will be performed.
  - `KEY1 = [TOKEN ...] FINAL_TOKEN` - Equivalent to `^KEY1 = [TOKEN ...] ^FINAL_TOKEN`, `~KEY1 = ~FINAL_TOKEN`. Example: `A = B` will make the A key act exactly like B.
  - `KEY1 = nothing` - Blocks the key. E.g. `DELETE = nothing`.
  - `... = KEY1 50ms KEY2` - A number with suffix ms, such as `50ms`, indicates a desired pause in milli-seconds.

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

# Acknowledgements

## Kmonad

Before developing this project, I used KMonad, which offered valuable insights into the design of keyboard remapping software. While this project diverges from KMonad in key ways, studying its structure influenced some of my initial design decisions. I’m grateful for the inspiration KMonad provided.

## Google

This project was initiated during my time at Google, though it was developed independently on personal equipment and is unrelated to Google’s core work. I would like to acknowledge Google for the opportunity to grow as a developer, which contributed to the skills I applied in this project.

# FAQ

## Will there be versions for Mac / Windows?

*Short answer: Not in the immediate future.*

While I have great respect for those operating systems, I do not use them myself, which makes it challenging for me to create and maintain releases for them. Therefore, there won’t be versions available in the foreseeable future.

However, I’d be open to considering this if someone is willing to take responsibility for setting up compilation for those OSes. Please note that I have some guidelines for contributions to ensure quality and consistency, so feel free to reach out if you’re interested!
