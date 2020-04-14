#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <iterator>
#include "json.hpp"

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
    std::string arg(argc == 2 || argc == 3 ? argv[1] : "");
    if(!((argc == 2 && (arg == "automated" || arg == "manual"))
        || (argc == 3 && arg == "manual"))){
        std::cerr << "Usage: " << argv[0] << " automated|manual <file>\n";
        return EXIT_FAILURE;
    }
    if(arg == "manual"){
        std::string input;
        if(argc == 3){
            std::ifstream in(argv[2], std::ios::in | std::ios::binary);
            if(!in){
                std::cerr << "Could not open file!\n";
                return EXIT_FAILURE;
            }
            in.seekg(0, std::ios::end);
            input.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&input[0], input.size());
            in.close();
        } else {
            // don't skip the whitespace while reading
              std::cin >> std::noskipws;
            // use stream iterators to copy the stream to a string
              std::istream_iterator<char> it(std::cin);
              std::istream_iterator<char> end;
              std::string results(it, end);
              input = results;
        }
        auto starttime = std::chrono::high_resolution_clock::now();
        hex::json j = hex::json::parse(input);
        double duration = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()-starttime).count();
        std::cout << (long long)(duration/1000000) << "ms\n";

        if(j.invalid()){
            std::cout << "Item is invalid!\n";
            std::cout << "At index " << (j.val.invalid_end - input.c_str()) << "\n";
            return EXIT_FAILURE;
        }
        // std::cout << j.dump() << '\n';
    } else if(arg == "automated"){
        //TODO: add tests
        hex::json j;
        j["a"] = "b";
        j["something"] = hex::json::make_arr({4, "a", 2.0});
        j["x"] = hex::json::make_obj({
            {"y", 7},
            {"z", "a"},
            {"extra", hex::json::make_arr({4, "a", 3}) }
        });
        std::cout << j.dump() << '\n';
    }
    return EXIT_SUCCESS;
}
