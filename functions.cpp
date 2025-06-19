
// === src/functions.cpp ===
#include "functions.h"
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

std::vector<Block> LoadDatabase(const std::string& directory) {
    std::vector<Block> blocks;
    for (const auto& entry : fs::directory_iterator(directory)) {
        std::ifstream file(entry.path());
        if (!file.is_open()) continue;

        Block block;
        std::string line;

        auto getValue = [](const std::string& line) -> std::string {
            size_t pos = line.find(":");
            if (pos == std::string::npos) return "";
            std::string value = line.substr(pos + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            return value;
        };

        std::getline(file, line); block.hash = getValue(line);
        std::getline(file, line); std::string heightStr = getValue(line);
        std::getline(file, line); std::string totalStr = getValue(line);
        std::getline(file, line); std::string timeStr = getValue(line);
        std::getline(file, line); block.relayed_by = getValue(line);
        std::getline(file, line); block.prev_block = getValue(line);

        try {
            block.height = std::stoi(heightStr);
            block.total = std::stol(totalStr);
            block.time = std::stol(timeStr);
        } catch (...) {
            continue;
        }

        blocks.push_back(block);
    }
    return blocks;
}

const Block* FindBlockByHash(const std::vector<Block>& blocks, const std::string& hash) {
    for (const auto& block : blocks) {
        if (block.hash == hash) {
            return &block;
        }
    }
    return nullptr;
}

const Block* FindBlockByHeight(const std::vector<Block>& blocks, int height) {
    for (const auto& block : blocks) {
        if (block.height == height) {
            return &block;
        }
    }
    return nullptr;
}

void PrintBlock(const Block& block) {
    std::cout << "hash: " << block.hash << "\n"
              << "height: " << block.height << "\n"
              << "total: " << block.total << "\n"
              << "time: " << block.time << "\n"
              << "relayed_by: " << block.relayed_by << "\n"
              << "prev_block: " << block.prev_block << "\n";
}

void PrintAllBlocks(const std::vector<Block>& blocks) {
    for (const auto& block : blocks) {
        PrintBlock(block);
        std::cout << "---------------------------\n";
    }
}

void ExportToCSV(const std::vector<Block>& blocks, const std::string& filename) {
    std::ofstream out(filename);
    out << "hash,height,total,time,relayed_by,prev_block\n";
    for (const auto& block : blocks) {
        out << block.hash << ',' << block.height << ',' << block.total << ','
            << block.time << ',' << block.relayed_by << ',' << block.prev_block << '\n';
    }
    std::cout << "Exported " << blocks.size() << " blocks to " << filename << std::endl;
}

void ReloadBlocksFromScript(const std::string& scriptPath, int count) {
    std::string command = "bash " + scriptPath + " " + std::to_string(count);
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Script execution failed with code: " << result << std::endl;
    }
}
