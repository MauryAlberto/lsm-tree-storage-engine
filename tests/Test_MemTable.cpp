#include <catch2/catch_test_macros.hpp>
#include "MemTable.hpp"

TEST_CASE("put function inserts and assigns", "[MemTable]") {
    MemTable table{10};
    REQUIRE(table.put("apple", "1") == true);
}

TEST_CASE("get function retrieves value of existing key", "[MemTable]") {
    MemTable table{10};
    table.put("apple", "1");
    REQUIRE(table.get("apple") == "1");
}

TEST_CASE("del function places tombstone value for an existing key", "[MemTable]") {
    MemTable table{10};
    table.put("apple", "1");
    REQUIRE(table.del("apple") == true);
    REQUIRE(table.get("apple") == "DELETED");
}

TEST_CASE("get returns std::nullopt on non existing key", "[MemTable]") {
    MemTable table{10};
    REQUIRE(table.get("apple") == std::nullopt);
}