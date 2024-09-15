#include <algorithm>
#include <functional>
#include <unordered_map>
#include <vector>

// Strings.

inline bool StartsWith(const std::string& str, const std::string& prefix) {
  return str.compare(0, prefix.length(), prefix) == 0;
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

// Sorted version of a map, with specified sort order; just by the key.
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
