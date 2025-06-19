// === main3_export_csv.cpp ===
#include "functions.h"
#include <iostream>

int main() {
    std::vector<Block> blocks = LoadDatabase("data");
    ExportToCSV(blocks, "output.csv");
    return 0;
}
