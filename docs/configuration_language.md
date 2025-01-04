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

## How to find keycodes

Run -
```sh
sudo evtest /dev/input/by-id/...-kbd
```

Then press any key that you want to know the id of.

All keycodes [/usr/include/linux/input-event-codes.h](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h) are supported. You can also look for a keycode there to map to, which is not available on your keyboard.

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

You will see a std::runtime_error, which is normal in this case and confirms deactivation.

