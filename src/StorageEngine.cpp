#include "StorageEngine.hpp"
#include "SSTableReader.hpp"
#include "SSTableWriter.hpp"

namespace lsmtse {
    std::optional<std::string> StorageEngine::get(const std::string &key)
    {
        const Entry* memEntry{memTable_.get(key)};    
        if(memEntry != nullptr) {
            if(memEntry->isTombstone) {
                return std::nullopt;
            }

            return memEntry->value;
        }

        for(auto it = sstablePaths_.rbegin(); it != sstablePaths_.rend(); it++) {
            SSTableReader reader{*it};
            auto result{reader.get(key)};
            if(result.has_value()) {
                if(result->isTombstone) {
                    return std::nullopt;
                }

                return result->value;
            }
        }

        return std::nullopt;
    }

    bool StorageEngine::flush()
    {
        if(memTable_.currentEntries() == 0) {
            return true;
        }

        std::filesystem::path sstablePath =
        dataDir_ / ("sstable_" + std::to_string(nextSSTableId_++) + ".log");

        SSTableWriter writer(sstablePath);
        for(const auto& [key, entry] : memTable_) {
            if(!writer.write(key, entry)) {
                return false;
            }
        }

        if(!writer.finish()) {
            return false;
        }

        sstablePaths_.push_back(sstablePath);
        memTable_.clear();
        wal_.clear();

        return true;
    }
}