// === main4_reload_blocks.cpp ===
#include "functions.h"
#include <iostream>

int main() {
    int count;
    std::cout << "Enter number of blocks to download: ";
    std::cin >> count;
    ReloadBlocksFromScript("fetch_blocks.sh", count);
    return 0;
}
