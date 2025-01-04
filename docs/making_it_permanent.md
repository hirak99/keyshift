# Make it Permanent for Your Keyboard

Keyshift is designed to clean up if it is interrupted or stopped.

Thus Keyshift must be running in order for the changes to remain in effect.

To make it permanent, you can start it on system boot, and keep it running in the background. This can be achieved in a few different ways.

Below are a few options to make Keyshift start and remain in the background with your keyboard and configuaration.

## Option 1. Systemd Service

Follow these steps -

- **Create a Config for your keyboard:** Create a config for your keyboard. Place it in `/opt/`.
  - You can see example [65perc.keyshift](../examples/65perc.keyshift) and modify it as per your needs.
  - You can refer to the [configuration language here](./configuration_language.md).
- **Create a Service:** Edit [keyshift.service](../examples/systemd/keyshift.service) (follow the comments), and place it in `/etc/systemd/system`.
- **Enable the Service:**
```sh
sudo systemctl enable --now keyshift
```

## Option 2. Udev

The advantage of doing this via udev is that it will activate automatically when you attach a particular keyboard.

- **Create a Config for your keyboard:** Follow the step in Option 1 to create a config and place it in `/opt/.
- **Create a udev rule:**
  - Modify the example [50-keyshift.rules](../examples/udev/50-keyshift.rules) following the comments there.
  - Place the file with your modifications in `/etc/udev/rules.d/`.
- **Activate the rules:**
  - The rules should activate next time you reboot.
  - You can activate the rules without rebooting, with `sudo udevadm control --reload-rules && sudo udevadm trigger`
