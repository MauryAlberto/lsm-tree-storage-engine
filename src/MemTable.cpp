#include "MemTable.hpp"

namespace lsmtse {
    const Entry* lsmtse::MemTable::get(std::string_view key)
    {
        auto it{table_.find(key)};
        if(it != table_.end()) {
            return &it->second;
        }

        return nullptr;
    }

    bool MemTable::del(std::string_view key)
    {
        if(!put(key, Entry{"", true})) {
            std::cerr << "deletion failed: table is full\n";
            return false;
        }

        return true;
    }
}
