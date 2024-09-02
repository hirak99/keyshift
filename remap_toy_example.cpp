#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <stdio.h>

void process_event(struct input_event *ie) {
    // Example remapping logic
    if (ie->type == EV_KEY) {
        if (ie->code == KEY_A && ie->value == 1) { // Key press
            // Remap KEY_A to KEY_B
            printf("Remapping A to B\n");
            // Instead of sending this event, you would send a new KEY_B event
        } else if (ie->code == KEY_A && ie->value == 0) { // Key release
            // Handle key release
        }
        // Add more remapping logic as needed
    }
}

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

