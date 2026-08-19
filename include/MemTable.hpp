#pragma once
#include <map>
#include <string>
#include <string_view>
#include <iostream>
#include <nlohmann/json.hpp>

namespace lsmtse {
    struct Entry {
        std::string value;
        bool isTombstone = false;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Entry, value, isTombstone);

    class MemTable {
        public:
            MemTable(size_t maxEntries) : maxEntries_(maxEntries) {}

            template <typename K, typename V>
            bool put(K&& key, V&& value) {
                auto it{table_.lower_bound(key)};

                if(it != table_.end() && !table_.key_comp()(key, it->first)) {
                    it->second = std::forward<V>(value);
                    return true;
                }

                if(isTableFull()) return false;

                table_.emplace_hint(it, std::forward<K>(key), std::forward<V>(value));
                currentEntries_++;
                return true;
            }

            const Entry* get(std::string_view key);
            bool del(std::string_view key);
            void clear();

            size_t currentEntries() { return currentEntries_; }
            bool isTableFull() const { return currentEntries_ >= maxEntries_; }
            auto begin() const { return table_.begin(); }
            auto end() const { return table_.end(); }

        private:
            size_t maxEntries_{0};
            size_t currentEntries_{0};
            std::map<std::string, Entry, std::less<>> table_;
    };
}
