#include "StorageEngine.hpp"
#include "SSTableReader.hpp"

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
}