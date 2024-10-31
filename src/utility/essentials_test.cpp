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

#include "essentials.h"

#include <catch2/catch_test_macros.hpp>
#include <string>

SCENARIO("StringSplit const char*") {
  CHECK(StringSplit("Hello World!", ",") ==
        std::vector<std::string>{"Hello World!"});
  CHECK(StringSplit("Hello,World!", ",") ==
        std::vector<std::string>{"Hello", "World!"});
  CHECK(StringSplit("Hello,,World!", ",") ==
        std::vector<std::string>{"Hello", "", "World!"});

  CHECK(StringSplit("0123456789", "53") ==
        std::vector<std::string>{"012", "4", "6789"});
  CHECK(StringSplit("0123456789", "43") ==
        std::vector<std::string>{"012", "", "56789"});

  CHECK(StringSplit("A=B;B=A", "\r\n;") ==
        std::vector<std::string>{"A=B", "B=A"});
  CHECK(StringSplit("A=B\nB=A", "\r\n;") ==
        std::vector<std::string>{"A=B", "B=A"});
  CHECK(StringSplit("A=B\r\nB=A", "\r\n;") ==
        std::vector<std::string>{"A=B", "", "B=A"});
}

SCENARIO("StringSplit char") {
  CHECK(StringSplit("Hello World!", ',') ==
        std::vector<std::string>{"Hello World!"});
  CHECK(StringSplit("Hello,World!", ',') ==
        std::vector<std::string>{"Hello", "World!"});
  CHECK(StringSplit("Hello,,World!", ',') ==
        std::vector<std::string>{"Hello", "", "World!"});
}
