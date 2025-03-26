# Configuration Language

## How to pass Configuration

Configuration can be passed in one of two ways.
1. `--config CONFIG` takes a string of config. This is useful for simple configurations. The statements can be separated by `;`.
2. `--config-file CONFIG_FILE.keyshift` takes a file as an argument. The file must be a text file, with configuration lines in it. It is customary to use the extension `.keyshift` for Keyshift config files.

### Example
Locate your keyboard in "/dev/input/by-id".

Then try this -

```sh
sudo keyshift "/dev/input/by-id/...-kbd" --config "A=B;B=A"
```

Verify that this swaps the keys A and B as long as it is running. You can exit out of it with Ctrl+C.

You can also load the configuration from a file instead with `--config-file /path/to/config.keyshift`.

Once you are happy with a configuration, you can add it to your startup. Or, you can add it to udev so that it activates whenever a particular keyboard is plugged in.

## Statement Reference

- Available keycodes -
  - You can use any keycode [/usr/include/linux/input-event-codes.h](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h). You can omit the "KEY_" prefix.
  - As special tokens, you can use `*` and `nothing`. See descriptions below.

- Lose syntax -
  - _(Basic)_ `KEY = [ACTION ...]`
    - `ACTION` can be one one of the following -
      - `x` (or `KEY_x`) - Where `KEY_x` is any [key code](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h) such as `KEY_F1`, indicates a press-and-release event.
      - `^x` (or `^KEY_x`) - Indicates just a press event.
      - `~x` (or `~KEY_x`) - Indicates just a release event.
      - `[num]ms` - indicates a pause of `[num]` milliseconds.
      - `nothing` - Blocks the key.
  - _(Layering)_ `KEY + TOKEN = [ACTION ...] | nothing | *`
    - `TOKEN` can be -
      - `nothing` - Indicates action to be taken if the activation key is pressed and released, with no other key pressed. Normally the layer absorbs the key. So `CAPSLOCK + 1 = F1; CAPSLOCK + nothing = CAPSLOCK` will make Capslock to behave as itself, unless any other key is press within it.
      - `x` (or `KEY_x`) - Any other specific key.
      - `*` indicating any key - In this case it must be of the form `KEY + * = *`. This will allow all keys to pass thru.
  - Implicit actions -
    - If a key activates a layer, releasing it will deactivate the layer, and generate release action for any keys pressed (but not yet released) due actions when it was held.
    - If a multiple layers are activated, keys will be modified through all layers.

### Examples

- Basic remapping -
  - `^A = ~B` - The `^` indicates press of a key. On KEY_A press, instead a KEY_B press will be performed.
  - `^A = ^B ~B` - The `~` indicates release of a key. On KEY_A press, a press and release of KEY_B will be performed.
  - `A = B` - Shorthand for `^A = ^B; ~A = ~B`.
  - `KEY1 = [ACTION ...] KEY_x` - Equivalent to `^KEY1 = [TOKEN ...] ^KEY_x`, `~KEY1 = ~KEY_x`. Example: `A = B` will make the A key act exactly like B.
  - `A = nothing` - Blocks the key. Pressing A will no longer be registered.
  - `A = ^H 50ms ~H 50ms I` - A number with suffix ms, such as `50ms`, indicates a desired pause in milli-seconds.

- Layering -
  - `A + B = [ACTIONS ...]` - Only if A is held, B will activate the actions.
    - Note that by default the layer-activation key, i.e. A in this example, will now be suppressed. If you need to also register the activation key, you must specify that explicitly.
        - `LEFTSHIFT + A = A` - Shift+A will result in "a".
        - `^LEFTSHIFT = ^LEFTSHIFT; LEFTSHIFT + A = A` - Shift+A will result in "A" since now shift also registers as is.
  - `KEY1 + KEY2 = *` - Shorthand for `KEY1 + KEY2 = KEY2`. Allows the key KEY2 to be passed thru in this layer unaltered.
  - `KEY1 + * = *` - Allow all keys not explicitly remapped under KEY1 to pass thru as is.
  - `KEY1 + nothing = [ACTION ...]` - Specifies what should happen if nothing inside the layer is activated. E.g. `DELETE + 1 = F1; DELETE + nothing = DELETE` will ensure DELETE acts as itself unless 1 is pressed within it.

## How to find keycodes

### Method 1.
All keycodes in [/usr/include/linux/input-event-codes.h](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h) are supported.

Keycodes start with `#define ...`. For example, `#define KEY_NUMLOCK ...`.

You should remove the prefix `KEY_`. For example, to refer to the key `#define KEY_NUMLOCK ...`, you should use just `NUMLOCK`. E.g. `keyshift ... --config "CAPSLOCK + N = NUMLOCK"`.

### Method 2.
Run -
```sh
sudo evtest /dev/input/by-id/...-kbd
```

Then press any key that you want to know the id of.

## Comments

You can use either `#` or `//` for line comments.

Block comments are not supported.

Example -

```
// This is a comment.

# This is also a comment.

A = B  // Make A act as the B key.
```

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

## Safety

If you lock yourself out due to a bad configuration, don't fret. There is a kill combo that deactivates keyshift. Type these keys in order while keyshift is running -

`KEYSHIFTRESERVEDCMDKILL`

Don't worry if you modified those keys with a configuration, the combo acts on actual keys.

You will notice the last `L` is not registered, and keyboard-remapper is terminated and deactivated from then on.
