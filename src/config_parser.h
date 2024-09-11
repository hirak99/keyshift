#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "keycode_lookup.h"
#include "remap_operator.h"

using std::string;

class ConfigParser {
 public:
  ConfigParser(Remapper* remapper);
  [[nodiscard]] bool Parse(const std::vector<string>& lines);

 private:
  // Converts a string like "~D ^A" to actions.
  std::vector<Action> AssignmentToActions(const string& assignment);

  std::vector<Action> AssignmentToActions(const std::vector<string>& tokens);

  bool ParseAssignment(const string& layer_name, const string& key_str,
                       const string& assignment);

  bool ParseLayerAssignment(const string& layer_key_str, const string& key_str,
                            const string& assignment);

  [[nodiscard]] bool ParseLine(const string& original_line);

  Remapper* remapper_;
  // To keep track of which layers have been seen. Used to do one time actions,
  // such as disallow other keys.
  std::set<string> known_layers_;
  // Only to display current line for errors.
  string line_being_parsed_;
};
