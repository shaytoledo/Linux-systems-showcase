// === main1_printdb.cpp ===
#include "functions.h"
#include <iostream>

int main() {
    std::vector<Block> blocks = LoadDatabase("data");
    PrintAllBlocks(blocks);
    return 0;
}
