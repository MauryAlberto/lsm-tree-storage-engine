#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include "StorageEngine.hpp"

using namespace lsmtse;

namespace {
    // keeps each test's data directory separate and cleans up after itself
    struct TempDataDir {
        std::filesystem::path path;

        explicit TempDataDir(const std::string& name)
                : path(std::filesystem::temp_directory_path() / name) {
            std::filesystem::remove_all(path);
            std::filesystem::create_directories(path);
        }

        ~TempDataDir() {
            std::filesystem::remove_all(path);
        }
    };
}

TEST_CASE("put() and get() work within a single engine instance", "[storage_engine]") {
    TempDataDir dir("se_test_basic");
    StorageEngine engine(dir.path, 100);

    REQUIRE(engine.put("key1", "value1"));
    auto result = engine.get("key1");

    REQUIRE(result.has_value());
    REQUIRE(result.value() == "value1");
}

TEST_CASE("get() returns nullopt for a key that was never written", "[storage_engine]") {
    TempDataDir dir("se_test_missing");
    StorageEngine engine(dir.path, 100);

    REQUIRE_FALSE(engine.get("does_not_exist").has_value());
}

TEST_CASE("remove() makes a key disappear from get()", "[storage_engine]") {
    TempDataDir dir("se_test_remove");
    StorageEngine engine(dir.path, 100);

    engine.put("key1", "value1");
    REQUIRE(engine.remove("key1"));

    REQUIRE_FALSE(engine.get("key1").has_value());
}

TEST_CASE("unflushed writes survive a restart via WAL replay", "[storage_engine]") {
    TempDataDir dir("se_test_wal_replay");

    {
        StorageEngine engine(dir.path, 100);   // capacity high, nothing will flush
        engine.put("key1", "value1");
        engine.put("key2", "value2");
    }   // engine destroyed here which simulates a crash/restart, no graceful shutdown

    StorageEngine restarted(dir.path, 100);
    auto r1 = restarted.get("key1");
    auto r2 = restarted.get("key2");

    REQUIRE(r1.has_value());
    REQUIRE(r1.value() == "value1");
    REQUIRE(r2.has_value());
    REQUIRE(r2.value() == "value2");
}

TEST_CASE("flushed SSTables and WAL-only writes both survive a restart, with deletes respected", "[storage_engine]") {
    TempDataDir dir("se_test_full_recovery");

    {
        StorageEngine engine(dir.path, 2);   // small capacity so we can force a real flush

        engine.put("key1", "value1");
        engine.put("key2", "value2");
        // memtable now full (2/2) so this put forces key1 and key2 to flush to sstable_0.log first
        engine.put("key3", "value3");
        // key3 now lives only in the memtable + WAL, not yet flushed

        // key1 was already flushed to an SSTable so delete it anyway
        engine.remove("key1");
    }   // "crash", engine destroyed with unflushed WAL data and one on-disk SSTable

    StorageEngine restarted(dir.path, 2);

    // key1: lived in an SSTable, but was deleted. Delete must survive the restart
    REQUIRE_FALSE(restarted.get("key1").has_value());

    // key2: only ever in the flushed SSTable. Found via SSTable discovery
    auto r2 = restarted.get("key2");
    REQUIRE(r2.has_value());
    REQUIRE(r2.value() == "value2");

    // key3: never flushed, recovered purely via WAL replay
    auto r3 = restarted.get("key3");
    REQUIRE(r3.has_value());
    REQUIRE(r3.value() == "value3");
}