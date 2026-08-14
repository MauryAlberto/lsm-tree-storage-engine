#pragma once
#include <fstream>
#include <filesystem>
#include "MemTable.hpp"

namespace lsmtse {
    class SSTableWriter {
        public:
            explicit SSTableWriter(std::filesystem::path filePath)
                : filePath_(std::move(filePath)), file_(filePath_,  std::ios::binary | std::ios::trunc) {
                    if(!file_) {
                            throw std::runtime_error("failed to open SSTable file: " + filePath_.string());
                    }
            }
            bool write(const std::string& key, const Entry& entry);
            bool finish();
        private:
            std::filesystem::path filePath_;
            std::ofstream file_;
    };
}