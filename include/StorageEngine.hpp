#pragma once
#include <filesystem>
#include <optional>
#include "MemTable.hpp"
#include "WriteAheadLog.hpp"

namespace lsmtse {
    class StorageEngine {
        public:
            StorageEngine(std::filesystem::path dataDir, size_t memTableMaxEntries);
            bool put(std::string key, std::string value);
            std::optional<std::string> get(const std::string& key);
            bool remove(const std::string& key);
        private:
            void flush();

            WriteAheadLog wal_;
            MemTable memTable_;
            std::vector<std::filesystem::path> sstablePaths_;
    };
}