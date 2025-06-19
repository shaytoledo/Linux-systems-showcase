// === main2_search.cpp ===
#include "functions.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " --hash <value> | --height <value>" << std::endl;
        return 1;
    }

    std::vector<Block> blocks = LoadDatabase("data");
    std::string flag = argv[1];
    std::string value = argv[2];

    if (flag == "--hash") {
        const Block* result = FindBlockByHash(blocks, value);
        if (result) {
            PrintBlock(*result);
                    } else {
            std::cout << "Block with hash " << value << " not found." << std::endl;
        }
    } else if (flag == "--height") {
        try {
            int height = std::stoi(value);
            const Block* result = FindBlockByHeight(blocks, height);
            if (result) {
                PrintBlock(*result);
                            } else {
                std::cout << "Block with height " << height << " not found." << std::endl;
            }
        } catch (...) {
            std::cerr << "Invalid height value." << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Unknown flag: " << flag << std::endl;
        return 1;
    }

    return 0;
}
