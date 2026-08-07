#pragma once
#include <map>
#include <string>
#include <string_view>

namespace lsmtse {
    struct Entry {
        std::string value;
        bool is_tombstone = false;
    };

    class MemTable {
        public:
            MemTable(size_t maxEntries) : max_entries_(maxEntries) {}

            template <typename K, typename V>
            bool put(K&& key, V&& value) {
                auto it{table_.lower_bound(key)};

                if(it != table_.end() && !table_.key_comp()(key, it->first)) {
                    it->second = std::forward<V>(value);
                    return true;
                }

                if(isTableFull()) return false;

                table_.emplace_hint(it, std::forward<K>(key), std::forward<V>(value));
                current_entries_++;
                return true;
            }

            const Entry* get(std::string_view key);
            bool del(std::string_view key);

            size_t currentEntries() { return current_entries_; }
            bool isTableFull() const { return current_entries_ > max_entries_; }

        private:
            size_t max_entries_{0};
            size_t current_entries_{0};
            std::map<std::string, Entry, std::less<>> table_;
    };
}
