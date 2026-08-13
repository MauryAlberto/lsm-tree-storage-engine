#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include "WriteAheadLog.hpp"

using namespace lsmtse;

namespace {
    // keeps each test's WAL file separate and cleans up after itself
    struct TempWalFile {
        std::filesystem::path path;

        explicit TempWalFile(const std::string& name)
                : path(std::filesystem::temp_directory_path() / name) {
            std::filesystem::remove(path);
        }

        ~TempWalFile() {
            std::filesystem::remove(path);
        }
    };
}

TEST_CASE("appendPut writes a record that recover() can read back", "[wal]") {
    TempWalFile temp("wal_test_append_put.log");
    WriteAheadLog wal(temp.path);

    REQUIRE(wal.appendPut("key1", Entry{"value1", false}));

    auto records = wal.recover();

    REQUIRE(records.size() == 1);
    REQUIRE(records[0].operation == OP::PUT);
    REQUIRE(records[0].key == "key1");
    REQUIRE(records[0].entry.value == "value1");
    REQUIRE(records[0].entry.isTombstone == false);
}

TEST_CASE("appendDelete writes a tombstone record", "[wal]") {
    TempWalFile temp("wal_test_append_delete.log");
    WriteAheadLog wal(temp.path);

    REQUIRE(wal.appendDelete("key1"));

    auto records = wal.recover();

    REQUIRE(records.size() == 1);
    REQUIRE(records[0].operation == OP::DELETE);
    REQUIRE(records[0].key == "key1");
    REQUIRE(records[0].entry.isTombstone == true);
}

TEST_CASE("multiple records recover in the order they were written", "[wal]") {
    TempWalFile temp("wal_test_multiple_records.log");
    WriteAheadLog wal(temp.path);

    wal.appendPut("key1", Entry{"value1", false});
    wal.appendPut("key2", Entry{"value2", false});
    wal.appendDelete("key1");

    auto records = wal.recover();

    REQUIRE(records.size() == 3);
    REQUIRE(records[0].key == "key1");
    REQUIRE(records[1].key == "key2");
    REQUIRE(records[2].operation == OP::DELETE);
}

TEST_CASE("recover on a fresh WAL file returns no records", "[wal]") {
    TempWalFile temp("wal_test_empty.log");
    WriteAheadLog wal(temp.path);

    auto records = wal.recover();

    REQUIRE(records.empty());
}

TEST_CASE("clear() empties the WAL so recover() returns nothing", "[wal]") {
    TempWalFile temp("wal_test_clear.log");
    WriteAheadLog wal(temp.path);

    wal.appendPut("key1", Entry{"value1", false});
    REQUIRE(wal.clear());

    auto records = wal.recover();
    REQUIRE(records.empty());
}

TEST_CASE("appending still works after clear()", "[wal]") {
    TempWalFile temp("wal_test_clear_then_append.log");
    WriteAheadLog wal(temp.path);

    wal.appendPut("key1", Entry{"value1", false});
    wal.clear();
    REQUIRE(wal.appendPut("key2", Entry{"value2", false}));

    auto records = wal.recover();

    REQUIRE(records.size() == 1);
    REQUIRE(records[0].key == "key2");
}

TEST_CASE("sync() succeeds after writing a record", "[wal]") {
    TempWalFile temp("wal_test_sync.log");
    WriteAheadLog wal(temp.path);

    wal.appendPut("key1", Entry{"value1", false});

    REQUIRE(wal.sync());
}