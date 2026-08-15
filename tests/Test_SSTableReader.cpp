#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include "SSTableWriter.hpp"
#include "SSTableReader.hpp"

using namespace lsmtse;

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
}

TEST_CASE("get() finds a key that was written", "[sstable]") {
    TempSSTableFile temp("sstable_reader_test_found.log");

    SSTableWriter writer(temp.path);
    writer.write("key1", Entry{"value1", false});
    writer.finish();

    SSTableReader reader(temp.path);
    auto result = reader.get("key1");

    REQUIRE(result.has_value());
    REQUIRE(result->value == "value1");
    REQUIRE(result->isTombstone == false);
}

TEST_CASE("get() returns nullopt for a key that was never written", "[sstable]") {
    TempSSTableFile temp("sstable_reader_test_missing.log");

    SSTableWriter writer(temp.path);
    writer.write("key1", Entry{"value1", false});
    writer.finish();

    SSTableReader reader(temp.path);
    auto result = reader.get("does_not_exist");

    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("get() finds the correct key among several", "[sstable]") {
    TempSSTableFile temp("sstable_reader_test_multiple.log");

    SSTableWriter writer(temp.path);
    writer.write("key1", Entry{"value1", false});
    writer.write("key2", Entry{"value2", false});
    writer.write("key3", Entry{"value3", false});
    writer.finish();

    SSTableReader reader(temp.path);
    auto result = reader.get("key2");

    REQUIRE(result.has_value());
    REQUIRE(result->value == "value2");
}

TEST_CASE("get() returns a tombstone entry for a deleted key", "[sstable]") {
    TempSSTableFile temp("sstable_reader_test_tombstone.log");

    SSTableWriter writer(temp.path);
    writer.write("key1", Entry{"", true});
    writer.finish();

    SSTableReader reader(temp.path);
    auto result = reader.get("key1");

    REQUIRE(result.has_value());
    REQUIRE(result->isTombstone == true);
}