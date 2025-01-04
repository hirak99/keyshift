# Quick Start

## What does this do?
This is a keyboard layering software.

Keyboard layering allows you to create multiple sets of key functions that can be accessed through different states or modes of the keyboard. This means that pressing a specific key or combination can switch to a new layer, enabling different key mappings and shortcuts without the need for additional physical keys. It enhances productivity and customization, making it easier to perform various tasks efficiently.

You can also see [here](remapping_needs.md) for some concrete examples of what it can do.

## Installing

### Archlinux
[Available in AUR](https://aur.archlinux.org/packages/keyshift), you can use your favorite AUR manager to install -

`yay -S keyshift`

### Others
```sh
git clone --recursive https://github.com/hirak99/keyshift
cd keyshift

./build_minimal.sh

# Test it locally.
./build/keyshift --help

# Optionally copy the binary to your system.
install -Dm 755 ./build/keyshift /usr/bin/keyshift
```

## Trying Out

Locate your keyboard in "/dev/input/by-id".

Then run this -

```sh
sudo keyshift "/dev/input/by-id/...-kbd" --config "A=B;B=A"
```

This will swap the A and B keys.

You can build more powerful config and store it in a file. When fully configured, this is intended to run in the background covering the session.
Ideally, you will start this on boot with a systemd service, or add it to udev so that it activates whenever a keyboard is plugged in.

For now you can exit pressing Ctrl+C.

