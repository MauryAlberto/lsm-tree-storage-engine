#include "WriteAheadLog.hpp"

bool lsmtse::WriteAheadLog::appendPut(std::string key, Entry entry)
{
    json j;
    j["op"] = OP::PUT;
    j["key"] = std::move(key);
    j["entry"] = entry;
    file_ << j.dump() << "\n";
    return !file_.fail();
}

bool lsmtse::WriteAheadLog::appendDelete(std::string key)
{
    json j;
    j["op"] = OP::DELETE;
    j["key"] = std::move(key);
    j["entry"] = Entry{"", true};
    file_ << j.dump() << "\n";
    return !file_.fail();
}

bool lsmtse::WriteAheadLog::sync()
{
    file_.flush();
    if(file_.fail()) {
        return false;
    }

    int fd = open(filePath_.c_str(), O_WRONLY);
    if(fd == -1) {
        std::cerr << "WAL sync: open failed: " << strerror(errno) << "\n";
        return false;
    }

    int status = fsync(fd);
    int savedError = errno;
    close(fd);
    if(status == -1) {
        std::cerr << "WAL sync: fsync failed: " << strerror(savedError) << "\n";
        return false;
    }

    return true;
}

bool lsmtse::WriteAheadLog::clear()
{
    file_.close();
    file_.open(filePath_, std::ios::trunc | std::ios::binary);
    if(file_.fail()) {
        return false;
    }
    file_.close();
    file_.open(filePath_, std::ios::app | std::ios::binary);
    if(file_.fail()) {
        return false;
    }

    return true;    
}

std::vector<lsmtse::WalRecord> lsmtse::WriteAheadLog::recover()
{
    file_.flush();

    std::vector<lsmtse::WalRecord> records;
    
    std::ifstream in(filePath_, std::ios::binary);
    if(!in.is_open()) {
        return records;
    }

    std::string line = "";
    while(getline(in, line)) {
        if(line.empty()) {
            continue;
        }

        try {
            json j = json::parse(line);
            records.emplace_back(j["op"], j["key"], j["entry"]);
        } catch (json::exception& e) {
            std::cerr << "WAL recover: skipping corrupt record: " << e.what() << "\n";
            skippedRecords_++;
            continue;
        }

    }

    return records;
}
