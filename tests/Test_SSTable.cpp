#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>
#include "SSTable.hpp"

using namespace lsmtse;
using json = nlohmann::json;

namespace {
    // keeps each test's SSTable file separate and cleans up after itself
    struct TempSSTableFile {
        std::filesystem::path path;

        explicit TempSSTableFile(const std::string& name)
                : path(std::filesystem::temp_directory_path() / name) {
            std::filesystem::remove(path);
        }

        ~TempSSTableFile() {
            std::filesystem::remove(path);
        }
    };

    // reads an SSTable file back manually, since SSTableReader doesn't exist yet
    std::vector<json> readRawRecords(const std::filesystem::path& path) {
        std::vector<json> records;
        std::ifstream in(path, std::ios::binary);
        std::string line;
        while(std::getline(in, line)) {
            if(line.empty()) {
                continue;
            }
            records.push_back(json::parse(line));
        }
        return records;
    }
}

TEST_CASE("write() produces a valid JSON line for a single entry", "[sstable]") {
    TempSSTableFile temp("sstable_test_single_write.log");
    SSTableWriter writer(temp.path);

    REQUIRE(writer.write("key1", Entry{"value1", false}));
    REQUIRE(writer.finish());

    auto records = readRawRecords(temp.path);
    REQUIRE(records.size() == 1);
    REQUIRE(records[0]["key"] == "key1");
    REQUIRE(records[0]["entry"]["value"] == "value1");
    REQUIRE(records[0]["entry"]["isTombstone"] == false);
}

TEST_CASE("multiple write() calls preserve insertion order", "[sstable]") {
    TempSSTableFile temp("sstable_test_multiple_writes.log");
    SSTableWriter writer(temp.path);

    REQUIRE(writer.write("key1", Entry{"value1", false}));
    REQUIRE(writer.write("key2", Entry{"value2", false}));
    REQUIRE(writer.write("key3", Entry{"", true}));
    REQUIRE(writer.finish());

    auto records = readRawRecords(temp.path);
    REQUIRE(records.size() == 3);
    REQUIRE(records[0]["key"] == "key1");
    REQUIRE(records[1]["key"] == "key2");
    REQUIRE(records[2]["key"] == "key3");
    REQUIRE(records[2]["entry"]["isTombstone"] == true);
}

TEST_CASE("finish() with no writes produces an empty file", "[sstable]") {
    TempSSTableFile temp("sstable_test_empty.log");
    SSTableWriter writer(temp.path);

    REQUIRE(writer.finish());

    auto records = readRawRecords(temp.path);
    REQUIRE(records.empty());
}