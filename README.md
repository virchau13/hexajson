# hexajson
Custom C++ json parser.

It is:
* Simple and easy to use (single file header include, uses OOP as to avoid global function clutter, only one class that is important)
* Small (655 line `json.hpp`)
* Fast (beats `nlohmann::json` on my i7-6700K, although it's not as fast as RapidJSON)

Requires C++17 or higher.

## Usage
Just put the file `json.hpp` anywhere in your project and `#include "json.hpp"`.

## Examples
To parse JSON:
```cpp
std::string input = get_file_input();
hex::json j = hex::json::parse(input);
```
To manipulate JSON objects:
```cpp
hex::json j = hex::json::make_obj({
    {"key", "value"},
    {"value", 0xca},
    {"n be anything really", hex::json::make_array({"arrays", "also", "are, "supported"}) }
});
j["key"] = "another value"; 
j["value"] = hex::json::make_array({ 3, 2, "a" });
```
