// Note that this can be dangerous if we grab the keyboard and make Ctrl+C
// impossible.
// So, to test, run this with `sudo timeout 20s ./<binary>`.
//
#include <linux/input.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>

#include "config_parser.h"
#include "input_device.h"
#include "keycode_lookup.h"
#include "remap_operator.h"
#include "virtual_device.h"

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

namespace po = boost::program_options;

[[nodiscard]] po::variables_map ParseArgs(int argc, char** argv) {
  po::options_description desc("Allowed options");
  desc.add_options()(
      "kbd", po::value<std::string>(),
      "Address of the -kbd device to remap in `/dev/input/by-path`.")(
      "config-string", po::value<std::string>(),
      "Config as a string, e.g. 'A=B;B=A'")(
      "config-file", po::value<std::string>(),
      "File with remapping configuration.")(
      "dry-run",
      "If passed, will not start a service but will only show previews.");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  return vm;
}

std::optional<std::string> GetOptionalArg(const po::variables_map& args,
                                          const std::string& name) {
  if (!args.count(name)) {
    return std::nullopt;
  }
  return args[name].as<std::string>();
}

std::string GetRequiredArg(const po::variables_map& args,
                           const std::string& name) {
  if (!args.count(name)) {
    std::cerr << "ERROR: Required argument " << name << " is missing."
              << std::endl;
    throw std::invalid_argument("Mandatory argument missing.");
  }
  return args[name].as<std::string>();
}

Remapper GetRemapper(std::optional<std::string> config,
                     std::optional<std::string> config_file) {
  std::vector<std::string> lines;
  if (config_file.has_value()) {
    std::ifstream file(config_file.value());
    if (!file.is_open()) {
      std::cerr << "ERROR: Could not open file " << config_file.value()
                << std::endl;
      exit(1);
    }
    std::string line;
    while (std::getline(file, line)) {
      lines.push_back(line);
    }
    file.close();
  }
  if (config.has_value()) {
    boost::algorithm::split(lines, config.value(), boost::is_any_of(";"));
  }

  Remapper remapper;
  ConfigParser config_parser(&remapper);
  if (!config_parser.Parse(lines)) {
    exit(1);
  }
  return remapper;
}

int main(int argc, char** argv) {
  auto args = ParseArgs(argc, argv);
  const bool arg_dry_run = args.count("dry-run") > 0;
  const std::string arg_kbd = GetRequiredArg(args, "kbd");
  const std::optional<std::string> arg_config =
      GetOptionalArg(args, "config-string");
  const std::optional<std::string> arg_config_file =
      GetOptionalArg(args, "config-file");

  Remapper remapper = GetRemapper(arg_config, arg_config_file);
  remapper.DumpConfig();
  VirtualDevice out_device;
  InputDevice device(arg_kbd.c_str());

  printf("Wating a sec, release all keys! ...\n");
  sleep(1);
  printf("Starting now...\n");

  if (arg_dry_run) {
    DisableEcho();
    auto echo_on_emit_fn = [](int key_code, int press) {
      std::cout << "  Out: " << (press ? "P " : "R ") << keyCodeToName(key_code)
                << std::endl;
    };
    remapper.SetCallback(echo_on_emit_fn);
    printf("Dryrun - processing disabled, echo enabled.\n");
  } else {
    remapper.SetCallback([&out_device](int code, int value) {
      out_device.DoKeyEvent(code, value);
    });
    device.Grab();
    printf("Processing enabled.\n");
  }

  struct input_event ie;
  while (read(device.get_fd(), &ie, sizeof(struct input_event)) > 0) {
    if (ie.type != EV_KEY) continue;

    if (arg_dry_run) {
      std::cout << "In: " << ie.value << " " << keyCodeToName(ie.code)
                << std::endl;
    }

    // This will call the function set with SetCallback() as new key events are
    // generated.
    remapper.Process(ie.code, ie.value);
  }
  return 0;
}
