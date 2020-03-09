#include <iostream>
#include <string>
#include <fstream>
#include <iterator>
#include "parser.hpp"

void test(int line, bool cond, bool ok){
    if(cond != ok){
        std::cerr << line << " test failed.\n";
        exit(EXIT_FAILURE);
    }
}

#define PASS(x) test(__LINE__, x, true)
#define FAIL(x) test(__LINE__, x, false)

/* Tests the JSON library. */
int main(int argc, char *argv[]){
    std::string arg(argc == 2 ? argv[1] : "");
    if(argc != 2 || (arg != "automated" && arg != "manual")){
        std::cerr << "Usage: " << argv[0] << " automated|manual\n";
        return EXIT_FAILURE;
    }
    if(arg == "manual"){
        std::cin >> std::noskipws;
        std::istream_iterator<char> start(std::cin), end;
        std::string input(start, end);
        hex::json j = hex::parse_json(input);
        std::cout << j.dump() << std::endl;
    } else if(arg == "automated"){
        //TODO: add tests
        hex::json j;
        j["a"] = "b";
        j["something"] = { 4, "a", 2.0 };
        j["x"] = hex::make_obj({
            {"y", 7},
            {"z", "a"},
            {"extra", { 4, "a", 3 } }
        });
        std::cout << j.dump() << '\n';
    }
    return EXIT_SUCCESS;
}
