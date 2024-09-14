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

// TODO: Move preview mode to command line --preview.
const bool kPreviewOnly = true;

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
      "File with remapping configuration.");
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
  const std::string arg_kbd = GetRequiredArg(args, "kbd");
  // TODO: Read either from config-string or from config-file.
  const std::optional<std::string> arg_config =
      GetOptionalArg(args, "config-string");
  const std::optional<std::string> arg_config_file =
      GetOptionalArg(args, "config-file");

  Remapper remapper = GetRemapper(arg_config, arg_config_file);
  remapper.DumpConfig();
  VirtualDevice out_device;
  InputDevice device(arg_kbd.c_str());
  if (!kPreviewOnly) device.Grab();

  printf("Wating a sec, release all keys! ...\n");
  sleep(1);
  printf("Starting now...\n");
  DisableEcho();

  if (kPreviewOnly) {
    auto echo_on_emit_fn = [](int key_code, int press) {
      std::cout << "  Out: " << (press ? "P " : "R ") << keyCodeToName(key_code)
                << std::endl;
    };
    remapper.SetCallback(echo_on_emit_fn);
  }

  struct input_event ie;
  while (read(device.get_fd(), &ie, sizeof(struct input_event)) > 0) {
    if (ie.type != EV_KEY) continue;

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
  return 0;
}
