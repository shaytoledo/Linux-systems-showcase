// === include/functions.h ===
#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

struct Block {
    std::string hash;
    int height;
    long total;
    long time;
    std::string relayed_by;
    std::string prev_block;
};

std::vector<Block> LoadDatabase(const std::string& directory);
const Block* FindBlockByHash(const std::vector<Block>& blocks, const std::string& hash);
const Block* FindBlockByHeight(const std::vector<Block>& blocks, int height);
void PrintBlock(const Block& block);
void PrintAllBlocks(const std::vector<Block>& blocks);
void ExportToCSV(const std::vector<Block>& blocks, const std::string& filename);
void ReloadBlocksFromScript(const std::string& scriptPath, int count);
