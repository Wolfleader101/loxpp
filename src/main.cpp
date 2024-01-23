#include <fstream>
#include <iostream>
#include <string>

void runCode(std::string& code);

void runFile(const char* path);
void runPrompt();

int main(int argc, char** argv) {
    if (argc > 2) {
        std::cout << "Usage: loxpp [script]" << std::endl;
        return 64;
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        runPrompt();
    }
}

void runFile(const char* path) {
    std::ifstream file_stream(path);

    if (file_stream.is_open()) {
        std::string file_contents((std::istreambuf_iterator<char>(file_stream)), (std::istreambuf_iterator<char>()));
        file_stream.close();

        runCode(file_contents);

    } else {
        std::cerr << "Error: Unable to open the file." << std::endl;
    }
}

void runPrompt() {
    for (;;) {
        std::cout << "> ";
        std::string line;
        std::getline(std::cin, line);

        if (std::cin.eof()) {
            std::cout << std::endl;
            break;
        }

        runCode(line);
    }
}

void runCode(std::string& code) {}