#include <catch2/catch_test_macros.hpp>
#include "MemTable.hpp"

TEST_CASE("put function inserts and assigns", "[MemTable]") {
    lsmtse::MemTable table{10};
    REQUIRE(table.put("apple", "1") == true);
}

TEST_CASE("get function retrieves value of existing key", "[MemTable]") {
    lsmtse::MemTable table{10};
    table.put("apple", "1");
    REQUIRE(table.get("apple")->value == "1");
}

TEST_CASE("del function places tombstone value for an existing key", "[MemTable]") {
    lsmtse::MemTable table{10};
    table.put("apple", "1");
    REQUIRE(table.del("apple") == true);
    REQUIRE(table.get("apple")->is_tombstone == true);
}

TEST_CASE("get returns std::nullopt on non existing key", "[MemTable]") {
    lsmtse::MemTable table{10};
    REQUIRE(table.get("apple") == nullptr);
}