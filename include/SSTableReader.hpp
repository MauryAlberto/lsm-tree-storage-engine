#pragma once

#include <optional>
#include <filesystem>
#include "MemTable.hpp"

namespace lsmtse {
    class SSTableReader {
        public:
            explicit SSTableReader(std::filesystem::path filePath)
                : filePath_(std::move(filePath)) {}
            std::optional<Entry> get(const std::string& key);
        private:
            std::filesystem::path filePath_;
    };
}