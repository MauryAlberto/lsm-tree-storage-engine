#pragma once
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdint>
#include <vector>
#include <nlohmann/json.hpp>
#include "MemTable.hpp"

using json = nlohmann::json;

namespace lsmtse {
    enum class OP {
        PUT,
        DELETE
    };

    struct WalRecord {
        OP operation;
        std::string key;
        lsmtse::Entry entry;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Entry, value, isTombstone);
    NLOHMANN_JSON_SERIALIZE_ENUM(OP, {
        {OP::PUT, "put"},
        {OP::DELETE, "delete"}
    });

    class WriteAheadLog {
        public:
            explicit WriteAheadLog(std::filesystem::path filePath)
                    : filePath_(std::move(filePath)), file_(filePath_, std::ios::app | std::ios::binary) {
                        if(!file_) {
                            throw std::runtime_error("failed to open WAL file: " + filePath_.string());
                        }
                    };
            bool appendPut(std::string key, Entry entry);
            bool appendDelete(std::string key);
            bool sync();
            bool clear();
            std::vector<WalRecord> recover();
            size_t skippedRecordCount() { return skippedRecords_; }
        private:
            std::filesystem::path filePath_;
            std::ofstream file_;
            size_t skippedRecords_{0};
    };
}