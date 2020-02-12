#include <iostream>
#include <string>
#include <fstream>
#include <iterator>
#include "parser.hpp"

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
        std::cout << j.dump_json() << std::endl;
    } else if(arg == "automated"){
        //TODO: add tests
    }
    return EXIT_SUCCESS;
}
