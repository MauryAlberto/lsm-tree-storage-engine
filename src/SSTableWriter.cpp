#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <nlohmann/json.hpp>
#include "SSTableWriter.hpp"

using json = nlohmann::json;

namespace lsmtse {
    bool lsmtse::SSTableWriter::write(const std::string &key, const Entry &entry)
    {
        json j;
        j["key"] = key;
        j["entry"] = entry;
        file_ << j.dump() << "\n";
        return !file_.fail();
    }

    bool lsmtse::SSTableWriter::finish()
    {
        file_.close();
        if(file_.fail()) {
            return false;
        }

        int fd = open(filePath_.c_str(), O_WRONLY);
        if(fd == -1) {
            std::cerr << "SSTable finish: open failed: " << strerror(errno) << "\n";
            return false;
        }

        int status = fsync(fd);
        int savedError = errno;
        close(fd);
        if(status == -1) {
            std::cerr << "SSTable finish: fsync failed: " << strerror(savedError) << "\n";
            return false; 
        }

        return true;
    }
}