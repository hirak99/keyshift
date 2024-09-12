// Note that this can be dangerous if we grab the keyboard and make Ctrl+C
// impossible.
// So, to test, run this with `sudo timeout 20s ./<binary>`.
//
// WIP
// - Take the keyboard device from comamnd line
// - Integrate with remapper, read from config
// - Move preview mode to command line --preview
#include <linux/input.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include <boost/program_options.hpp>
#include <iostream>

#include "input_device.h"
#include "keycode_lookup.h"
#include "remap_operator.h"
#include "virtual_device.h"

const bool kPreviewOnly = true;

// Disable echoing input when run in terminal.
void DisableEcho() {
  struct termios tty;
  if (tcgetattr(STDIN_FILENO, &tty) != 0) {
    perror("tcgetattr");
    return;
  }
  tty.c_lflag &= ~ECHO;  // Disable echo
  if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) != 0) {
    perror("tcsetattr");
  }
}

namespace po = boost::program_options;

[[nodiscard]] po::variables_map ParseArgs(int argc, char **argv) {
  po::options_description desc("Allowed options");
  desc.add_options()(
      "kbd", po::value<std::string>(),
      "Address of the -kbd device to remap in `/dev/input/by-path`.")(
      "config", po::value<std::string>(), "File with remapping configuration.");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  // std::cout << vm["kbd"].as<std::string>() << std::endl;
  return vm;
}

int main(int argc, char **argv) {
  auto args = ParseArgs(argc, argv);
  if (!args.count("kbd")) {
    std::cerr << "Error: kbd is required." << std::endl;
    return 1;
  }

  const std::string arg_kbd = args["kbd"].as<std::string>();

  printf("Wating a sec, release all keys! ...\n");
  sleep(1);
  printf("Starting now...\n");
  DisableEcho();

  InputDevice device(arg_kbd.c_str());
  if (!kPreviewOnly) device.Grab();

  Remapper remapper;
  remapper.AddMapping("", KeyPressEvent(KEY_A),
                      {remapper.ActionActivateState("fn_layer")});
  remapper.AddMapping("fn_layer", KeyPressEvent(KEY_S), {KeyPressEvent(KEY_B)});
  remapper.AddMapping("fn_layer", KeyReleaseEvent(KEY_S),
                      {KeyReleaseEvent(KEY_B)});
  VirtualDevice out_device;

  if (kPreviewOnly) {
    auto echo_on_emit_fn = [](int key_code, int press) {
      std::cout << "  Out: " << (press ? "P " : "R ") << keyCodeToName(key_code)
                << std::endl;
    };
    remapper.SetCallback(echo_on_emit_fn);
  }

  struct input_event ie;
  while (read(device.get_fd(), &ie, sizeof(struct input_event)) > 0) {
    if (ie.type == EV_KEY) {
      // printf("Key %i %s\n", ie.code, ie.value ? "pressed" : "released");

      if (kPreviewOnly) {
        std::cout << "In: " << ie.value << " " << keyCodeToName(ie.code)
                  << std::endl;
      }

      remapper.Process(ie.code, ie.value);

      if (!kPreviewOnly) {
        // Process and remap key events here
        if (ie.value) {
          out_device.KeyPress(ie.code);
        } else {
          out_device.KeyRelease(ie.code);
        }
      }
    }
  }
  return 0;
}
