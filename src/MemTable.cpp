#include "MemTable.hpp"

std::optional<std::string_view> MemTable::get(std::string_view key)
{
    auto it{table_.find(key)};
    if(it != table_.end()) {
        return it->second;
    }

    return std::nullopt;
}

bool MemTable::del(std::string_view key)
{
    auto it{table_.find(key)};
    if(it != table_.end()) {
        if(!put(key, "DELETED")) {
            printf("deletion failed: table is full\n");
            return false;
        }

        return true;
    }

    printf("deletion failed: key not found\n");
    return false;
}