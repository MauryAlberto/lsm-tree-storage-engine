#include "WriteAheadLog.hpp"

bool lsmtse::WriteAheadLog::appendPut(std::string key, Entry entry)
{
    json j;
    j["op"] = OP::PUT;
    j["key"] = std::move(key);
    j["entry"] = entry;
    file_ << j.dump();
    return !file_.fail();
}

bool lsmtse::WriteAheadLog::appendDelete(std::string key)
{
    json j;
    j["op"] = OP::DELETE;
    j["key"] = std::move(key);
    j["entry"] = Entry{"", true};
    file_ << j.dump();
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
        return false;
    }

    int status = fsync(fd);
    close(fd);
    if(status == -1) {
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
    return std::vector<WalRecord>();
}
