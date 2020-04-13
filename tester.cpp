#include <iostream>
#include <string>
#include <fstream>
#include <iterator>
#include "json.hpp"
#include <filesystem>
namespace fs = std::filesystem;

std::string read_file(const std::string& path){
    std::string input;
    std::ifstream in(path, std::ios::in | std::ios::binary);
    in.seekg(0, std::ios::end);
    input.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&input[0], input.size());
    in.close();
    return input;
}

std::string red = "\033[1;31m", green = "\033[1;32m", norm = "\033[0m";

/* Tests the JSON library. */
int main(int argc, char *argv[]){
    int passc = 0, failc = 0, num = 0;
    std::cout << "Fail testing...\n";
    for(const auto& fail : fs::directory_iterator("./fail/")){
        hex::json j = hex::json::parse(read_file(fail.path()));
        if(j.invalid()){
            passc++;
        } else {
            std::cout << red << fail.path().filename() << " failed\n" << norm;
            failc++;
        }
        num++;
    }
    std::cout << "Fail test results: Out of " << num << " tests, ";
    if(passc) std::cout << green << passc << " tests passed"; 
    if(failc) {
        if(passc) std::cout << ", ";
        std::cout << red << failc << " tests failed";
    }
    std::cout << '\n' << norm;
    std::cout << "Pass testing...\n";
    passc = failc = 0;
    for(const auto& pass : fs::directory_iterator("./pass/")){
        hex::json j = hex::json::parse(read_file(pass.path()));
        if(!j.invalid()){
            passc++;
        } else {
            std::cout << red << pass.path().filename() << " failed\n" << norm;
            failc++;
        }
    }
    std::cout << "Pass test results: Out of " << num << " tests, ";
    if(passc) std::cout << green << passc << " tests passed";
    if(failc){
        if(passc) std::cout << ", ";
        std::cout << red << failc << " tests failed";
    }
    std::cout << '\n' << norm;
}
