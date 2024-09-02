#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    int fd = open("/dev/input/by-id/usb-Drunkdeer_Drunkdeer_G65_US_RYMicro-event-kbd", O_RDONLY);
    if (fd < 0) {
        perror("Unable to open input device");
        return 1;
    }

    struct input_event ie;
    while (read(fd, &ie, sizeof(struct input_event)) > 0) {
        if (ie.type == EV_KEY) {
            printf("Key %i %s\n", ie.code, ie.value ? "pressed" : "released");
            // Process and remap key events here
        }
    }

    close(fd);
    return 0;
}

