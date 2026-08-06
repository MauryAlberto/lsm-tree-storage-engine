#include "MemTable.hpp"

const lsmtse::Entry* lsmtse::MemTable::get(std::string_view key)
{
    auto it{table_.find(key)};
    if(it != table_.end()) {
        return &it->second;
    }

    return nullptr;
}

bool lsmtse::MemTable::del(std::string_view key)
{
    auto it{table_.find(key)};
    if(it != table_.end()) {
        if(!put(key, Entry{"", true})) {
            printf("deletion failed: table is full\n");
            return false;
        }

        return true;
    }

    printf("deletion failed: key not found\n");
    return false;
}
