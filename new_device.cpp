#include <fcntl.h>
#include <linux/uinput.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int setup_uinput() {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("Unable to open /dev/uinput");
        return -1;
    }

    // Set up the uinput device
    struct uinput_setup setup;
    memset(&setup, 0, sizeof(setup));
    setup.id.bustype = BUS_USB;
    setup.id.vendor = 0x1234;
    setup.id.product = 0x5678;
    setup.id.version = 1;
    strcpy(setup.name, "My Virtual Keyboard");

    // Enable the necessary event types and keys
    if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0) {
        perror("UI_SET_EVBIT failed");
        close(fd);
        return -1;
    }
    if (ioctl(fd, UI_SET_KEYBIT, KEY_A) < 0) {
        perror("UI_SET_KEYBIT failed");
        close(fd);
        return -1;
    }

    if (ioctl(fd, UI_DEV_SETUP, &setup) < 0) {
        perror("UI_DEV_SETUP failed");
        close(fd);
        return -1;
    }

    if (ioctl(fd, UI_DEV_CREATE) < 0) {
        perror("UI_DEV_CREATE failed");
        close(fd);
        return -1;
    }

    return fd;
}

void send_event(int fd, unsigned int type, unsigned int code, int value) {
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = type;
    ev.code = code;
    ev.value = value;

    if (write(fd, &ev, sizeof(ev)) < 0) {
        perror("write failed");
    }
}

int main() {
    int uinput_fd = setup_uinput();
    if (uinput_fd < 0) {
        return 1;
    }

    // Send a key press event for KEY_A
    send_event(uinput_fd, EV_KEY, KEY_A, 1); // Key press
    send_event(uinput_fd, EV_SYN, SYN_REPORT, 0); // Synchronize

    sleep(1); // Wait for a second

    // Send a key release event for KEY_A
    send_event(uinput_fd, EV_KEY, KEY_A, 0); // Key release
    send_event(uinput_fd, EV_SYN, SYN_REPORT, 0); // Synchronize

    // Clean up and destroy the uinput device
    if (ioctl(uinput_fd, UI_DEV_DESTROY) < 0) {
        perror("UI_DEV_DESTROY failed");
    }
    close(uinput_fd);

    return 0;
}
