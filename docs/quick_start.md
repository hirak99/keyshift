# Quick Start

## Examples
You can check [this doc](remapping_needs.md) and [this example config](../examples/65perc.keyshift) to get a taste of what this can do.

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

## Trying

Locate your keyboard in "/dev/input/by-id".

Then run this -

```sh
sudo keyshift "/dev/input/by-id/...-kbd" --config "A=B;B=A"
```

This will swap the A and B keys.

You can build more powerful config and store it in a file. When fully configured, this is intended to run in the background covering the session.
Ideally, you will start this on boot with a systemd service, or add it to udev so that it activates whenever a keyboard is plugged in.

You can exit pressing Ctrl+C.

Note that once you exit, the changes will no longer remain in effect. To make it permanent, you can [follow this guide](./making_it_permanent.md).
