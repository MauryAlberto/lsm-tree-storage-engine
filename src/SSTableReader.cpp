#include "SSTableReader.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace lsmtse {
    std::optional<lsmtse::Entry> lsmtse::SSTableReader::get(const std::string &key)
    {
        std::ifstream file(filePath_);
        if(!file) {
                throw std::runtime_error("failed to open SSTable file: " + filePath_.string());
        }

        int lineNumber = 1;
        std::string line = "";
        while(getline(file, line)) {
            if(line.empty()) {
                lineNumber++;
                continue;
            }

            try {
                json j = json::parse(line);

                if(j.contains("key") && j["key"] == key && j.contains("entry")) {
                    lsmtse::Entry entry = j["entry"];
                    return entry;
                }
            } catch(const json::parse_error& e) {
                std::cerr << "parse error at line " << lineNumber << ": " << e.what() << std::endl;
            }

            lineNumber++;
        }

        return std::nullopt;
    }
}