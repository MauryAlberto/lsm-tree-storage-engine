#pragma once
#include <filesystem>
#include <optional>
#include <charconv>
#include <vector>
#include <utility>
#include <algorithm>
#include "MemTable.hpp"
#include "WriteAheadLog.hpp"

namespace lsmtse {
    class StorageEngine {
        public:
            StorageEngine(std::filesystem::path dataDir, size_t memTableMaxEntries)
            : dataDir_(dataDir),
              wal_(dataDir_ / "wal.log"),
              memTable_(memTableMaxEntries)
              {
                std::vector<WalRecord> records = wal_.recover();

                for(const auto& record : records) {
                    memTable_.put(record.key, record.entry);
                }

                std::vector<std::pair<size_t, std::filesystem::path>> sstablesFound;
                std::string target = "sstable_";
                
                if(std::filesystem::exists(dataDir_) && std::filesystem::is_directory(dataDir_)) {
                    for(const auto& entry : std::filesystem::directory_iterator(dataDir_)) {
                        size_t position = entry.path().filename().string().find(target);
                        
                        if(position != std::string::npos) {
                            size_t value = 0;
                            std::string sstableFile = entry.path().filename().string();
                            size_t numberStart = position + target.size();
                            auto [ptr, ec] = std::from_chars(sstableFile.data() + numberStart, sstableFile.data() + sstableFile.size(), value);
                            
                            if(ec == std::errc{}) {
                                sstablesFound.emplace_back(value, entry.path());
                            } else if(ec == std::errc::invalid_argument) {
                                std::cerr << "Storage Engine: not a valid number\n";
                            }
                        }
                    }

                    std::sort(sstablesFound.begin(), sstablesFound.end());
                    
                    for(const auto& pair : sstablesFound) {
                        sstablePaths_.push_back(pair.second);
                    }

                    nextSSTableId_ = sstablesFound.empty() ? 0 : sstablesFound.back().first + 1;
                } else {
                    std::cerr << "Storage Engine: path does not exist or is not a directory\n";
                }
              }
              
            bool put(std::string key, std::string value);
            std::optional<std::string> get(const std::string& key);
            bool remove(const std::string& key);
        private:
            bool flush();

            std::filesystem::path dataDir_;
            WriteAheadLog wal_;
            MemTable memTable_;
            std::vector<std::filesystem::path> sstablePaths_;
            size_t nextSSTableId_{0};
    };
}