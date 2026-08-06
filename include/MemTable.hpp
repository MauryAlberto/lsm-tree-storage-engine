#pragma once
#include <map>
#include <string>
#include <optional>
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
                if(isTableFull()) return false;

                auto [it, inserted] = table_.insert_or_assign(
                    typename decltype(table_)::key_type(std::forward<K>(key)),
                    typename decltype(table_)::mapped_type(std::forward<V>(value))
                );

                if(inserted) current_entries_++;
                return true;
            }

            const Entry* get(std::string_view key);
            bool del(std::string_view key);

            size_t currentEntries() { return current_entries_; }
            bool isTableFull() const { return current_entries_ >= max_entries_; }

        private:
            size_t max_entries_{0};
            size_t current_entries_{0};
            std::map<std::string, Entry, std::less<>> table_;
    };
}
