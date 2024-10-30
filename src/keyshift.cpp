// Note that this can be dangerous if we grab the keyboard and make Ctrl+C
// impossible.
// So, to test, run this with `sudo timeout 20s ./<binary>`.
//
#include <linux/input.h>
#include <poll.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include <atomic>
#include <boost/algorithm/string.hpp>
#include <csignal>
#include <expected>
#include <fstream>
#include <iostream>

#include "config_parser.h"
#include "input_device.h"
#include "keycode_lookup.h"
#include "remap_operator.h"
#include "utility/argparse.h"
#include "utility/every_n_ms.h"
#include "utility/os_level_mutex.h"
#include "version.h"
#include "virtual_device.h"

// How long to poll for reads before looking for interruptions.
const int kReadTimeoutMS = 1500;

// Set to true on interrupts.
std::atomic<bool> kInterrupted(false);

// Disable echoing input when run in terminal.
void DisableEcho() {
  struct termios tty;
  if (tcgetattr(STDIN_FILENO, &tty) != 0) {
    perror("tcgetattr");
    return;
  }
  tty.c_lflag &= ~ECHO;  // Disable echo on terminal.
  if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) != 0) {
    perror("tcsetattr");
  }
}

std::optional<ArgumentParser> ParseArgs(const int argc, const char** argv) {
  ArgumentParser parser;
  parser.AddBool("help", "Show a short help.");
  parser.AddString(
      "kbd", "Address of the -kbd device to remap in `/dev/input/by-path/`.");
  parser.AddString("config",
                   "Config as a semi-colon delimited strings, e.g. 'A=B;B=A'.");
  parser.AddString("config-file", "File with remapping configuration.");
  parser.AddBool(
      "dump", "Show internal representation of the parsed config, and exit.");
  parser.AddBool(
      "dry-run",
      "If passed, will not start a service but will only show previews.");
  parser.AddBool("version", "Display commit id and exit.");

  parser.Parse(argc, argv);

  const bool arg_help = parser.GetBool("help");
  if (arg_help) {
    parser.ShowHelp();
    return std::nullopt;
  }
  const bool arg_version = parser.GetBool("version");
  if (arg_version) {
    std::cout << "Commit id: " << GIT_COMMIT_ID << std::endl;
    std::cout << "Commit time: " << GIT_COMMIT_TIME << std::endl;
    return std::nullopt;
  }
  return parser;
}

std::expected<Remapper, std::string> GetRemapper(
    const std::optional<std::string>& config,
    const std::optional<std::string>& config_file) {
  std::vector<std::string> lines;
  if (config_file.has_value()) {
    std::ifstream file(config_file.value());
    if (!file.is_open()) {
      return std::unexpected("Could not open file " + config_file.value());
    }
    std::string line;
    while (std::getline(file, line)) {
      lines.push_back(line);
    }
    file.close();
  }
  if (config.has_value()) {
    boost::algorithm::split(lines, config.value(), boost::is_any_of(";\r\n"));
  }

  Remapper remapper;
  ConfigParser config_parser(&remapper);
  if (!config_parser.Parse(lines)) {
    return std::unexpected("Failed to parse file");
  }
  return remapper;
}

void SignalHandler(const int signum) {
  std::cerr << "Interruption signal (" << signum << ") received, terminating."
            << std::endl;
  kInterrupted.store(true);
}

int MainLoop(InputDevice& device, Remapper& remapper, bool echo_inputs) {
  // Set up handlers which will set kInterrupded on any error.
  std::signal(SIGINT, SignalHandler);
  std::signal(SIGTERM, SignalHandler);
  std::signal(SIGHUP, SignalHandler);

  // Most of the mess below is to set up timeouts. Had we not needed that, we'd
  // just change the if to while and put the kInterrupted detection within it.
  // However, without this, SIGTERM will wait indefinitely during poweroff until
  // a key is pressed - we don't want that.
  const int fd = device.get_fd();

  struct pollfd fds[1];
  fds[0].fd = fd;
  fds[0].events = POLLIN;

  struct input_event ie;

  while (true) {
    // Gracefully exit on interruption.
    if (kInterrupted.load()) [[unlikely]]
      return 2;

    const int poll_ret = poll(fds, 1, kReadTimeoutMS);
    switch (poll_ret) {
      [[unlikely]] case -1:
        // This can also occur when the interrupt happens amidst system call.
        // perror(x) will show "x: Interrupted system call".
        perror("ERROR reading device");
        return 1;
      case 0:
        // Timeout.
        break;
      default:
        // There is data to be read, and the read is no longer blocking.
        if (read(fd, &ie, sizeof(struct input_event)) > 0) [[likely]] {
          if (ie.type != EV_KEY) continue;

          if (echo_inputs) [[unlikely]] {
            std::cout << "In: ";
            std::cout << (ie.value == 1   ? "P "
                          : ie.value == 0 ? "R "
                                          : "T ")
                      << KeyCodeToName(ie.code);
            std::cout << std::endl;
          }

          // This will call the function set with SetCallback() as new key
          // events are generated.
          remapper.Process(ie.code, ie.value);
        } else [[unlikely]] {
          // Happens at an alarming rate sometimes!
          // Counted 1102381 lines in log in a few minites.
          // EVEY_N_MS ensures we do not spam the journal.
          EVERY_N_MS_W_SUPPRESSED(500, perror("Failed read"));
        }
    }
  }
}

int main(const int argc, const char** argv) {
  auto args_opt = ParseArgs(argc, argv);
  if (!args_opt) return 0;
  auto args = args_opt.value();
  const bool arg_dump = args.GetBool("dump");
  const bool arg_dry_run = args.GetBool("dry-run");
  const std::optional<std::string> arg_config = args.GetString("config");
  const std::optional<std::string> arg_config_file =
      args.GetString("config-file");

  auto remapper_exc = GetRemapper(arg_config, arg_config_file);
  if (!remapper_exc) {
    std::cerr << "ERROR: " << remapper_exc.error() << std::endl;
    return EXIT_FAILURE;
  }
  Remapper remapper = std::move(remapper_exc.value());
  if (arg_dump) {
    remapper.DumpConfig();
    return EXIT_SUCCESS;
  }

  const std::string arg_kbd = args.GetRequiredString("kbd");
  std::optional<OSMutex> mutex;
  // Use mutex only if this is not dry-run and we intend to grab the device.
  if (!arg_dry_run) {
    mutex = AcquireOSMutex("keyshift_" + arg_kbd);
    if (!mutex) {
      std::cerr << "Another instance is starting for specified kbd, exiting."
                << std::endl;
      return EXIT_FAILURE;
    }
  }
  InputDevice device(arg_kbd.c_str());
  VirtualDevice out_device;

  if (arg_dry_run) {
    DisableEcho();
    auto echo_on_emit_fn = [](int key_code, int press) {
      std::cout << "  Out: ";
      std::cout << (press == 1   ? "P "
                    : press == 0 ? "R "
                                 : "T ")
                << KeyCodeToName(key_code);
      std::cout << std::endl;
    };
    remapper.SetCallback(echo_on_emit_fn);
    printf("Dryrun - processing disabled, echo enabled.\n");
  } else {
    remapper.SetCallback([&out_device](int code, int value) {
      out_device.DoKeyEvent(code, value);
    });
    device.Grab();
    // Preserve the mutex only until a device has been grabbed.
    // This helps to not maintain the file in /dev/shm.
    // Also it is sufficeint to ensure if multiple calls happen during
    // initialization, e.g. because of udev rules matching multiple times, they
    // are blocked.
    mutex.reset();
    printf("Processing enabled.\n");
  }

  // Control returns from MainLoop only if interrupted or killed.
  return MainLoop(device, remapper, arg_dry_run);
}
