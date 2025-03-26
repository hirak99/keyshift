/*
 * Copyright (c) 2024 Nomen Aliud (aka Arnab Bose)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __ESSENTIALS_H
#define __ESSENTIALS_H

#include <algorithm>
#include <expected>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Strings.

inline std::string ToLowerCase(const std::string& str) {
  std::string result = str;
  for (char& c : result) {
    c = std::tolower(static_cast<unsigned char>(c));
  }
  return result;
}

inline bool StartsWith(const std::string& str, const std::string& prefix) {
  return str.compare(0, prefix.length(), prefix) == 0;
}

// Split by one or more characters.
// Replacement for boost::algorithms::split() with boost::anyof.
inline std::vector<std::string> StringSplit(const std::string& str,
                                            const char* delimiters) {
  std::vector<std::string> tokens;
  std::string token;
  std::size_t start = 0;
  std::size_t end;

  while (start < str.size()) {
    end = str.find_first_of(delimiters, start);

    // If no more delimiters are found, take the rest of the string.
    if (end == std::string::npos) {
      tokens.push_back(str.substr(start));
      break;
    }

    tokens.push_back(str.substr(start, end - start));
    start = end + 1;
  }

  return tokens;
}

inline std::vector<std::string> StringSplit(const std::string& str,
                                            const char delimiter) {
  // Uhh... using char instead of string. Check this carefully.
  // Justified since str.find_first_of() works with character array.
  char delimiters[2];
  delimiters[0] = delimiter;
  delimiters[1] = 0;
  return StringSplit(str, delimiters);
}

// Let's bring some sanity to maps.

// Map contains.
template <typename T, typename U>
bool MapContains(const std::unordered_map<T, U>& map, const T& key) {
  const auto& it = map.find(key);
  return it != map.end();
}

// Map lookup.
template <typename T, typename U>
std::optional<U> MapLookup(const std::unordered_map<T, U>& map, const T& key) {
  const auto& it = map.find(key);
  if (it == map.end()) return std::nullopt;
  return it->second;
}

// Sorted version of a map, with specified sort order.
// E.g. -
// for (const auto& [key, value] :
//      Sorted(all_states_, [](const std::string& value1,
//                             const std::string& value2) {
//        return value1 < value2;
//      })) {
//   ...
// }
template <typename T, typename U, typename Func>
std::vector<std::pair<T, U>> Sorted(const std::unordered_map<T, U>& input,
                                    Func value_ordering) {
  std::vector<std::pair<T, U>> result;
  result.insert(result.begin(), input.begin(), input.end());
  std::sort(
      result.begin(), result.end(),
      [value_ordering](const std::pair<T, U>& el1, const std::pair<T, U>& el2) {
        return el1.first < el2.first ||
               (el1.first == el2.first &&
                value_ordering(el1.second, el2.second));
      });

  return result;
}

// Sorted version of a map, sorted by the key.
template <typename T, typename U>
std::vector<std::pair<T, U>> Sorted(const std::unordered_map<T, U>& input) {
  std::vector<std::pair<T, U>> result;
  result.insert(result.begin(), input.begin(), input.end());
  std::sort(result.begin(), result.end(),
            [](const std::pair<T, U>& el1, const std::pair<T, U>& el2) {
              return el1.first < el2.first;
            });
  return result;
}

// Convenience macro for std::except error handling.

// Provides brevity in place of std::expected<..., std::string> for brevity.
template <typename T>
using ErrorStrOr = std::expected<T, std::string>;

// Defines only active during static checking. Not compiled.
#if __INTELLISENSE__
// Avoid Visual Studio Code marking GCC defines as errors. See -
// https://github.com/microsoft/vscode-cpptools/issues/11164#issuecomment-2115945518

// Note: __FILE_NAME__ is the basename, __FILE__ is the full name that can leak
// information. However this define is only active for static checking.
#define __FILE_NAME__ __FILE__
#endif

// Similar to value() which throws exception, except that this propagates the
// exception to the calling function.
// E.g. -
// std::except<void, std::string> test(float a) {
//   ASSIGN_OR_RETURN(const float x, sqrt_or_error(a));
//   std::cout << "Result: " << x << std::endl;
//   return {};
// }
#define ASSIGN_OR_RETURN(result, result_as_expected)                \
  const auto temp_##__LINE__ = result_as_expected;                  \
  if (!temp_##__LINE__) {                                           \
    return std::unexpected(std::string("(") + __FILE_NAME__ + ":" + \
                           std::to_string(__LINE__) + ") " +        \
                           temp_##__LINE__.error());                \
  }                                                                 \
  result = *temp_##__LINE__;

#endif  // __ESSENTIALS_H
