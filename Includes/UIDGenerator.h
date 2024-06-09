#include <cstdint>
#include <atomic>
#include <ctime>
#include <mutex>

class UIDGenerator {

private:
    UIDGenerator() = default;
    UIDGenerator(const UIDGenerator&) = delete;
    UIDGenerator& operator=(const UIDGenerator&) = delete;
    
    const int TIMESTAMP_BITS = 32;
    std::atomic<uint64_t> counter{0};

    static uint64_t getCurrentTimestamp() { return static_cast<uint64_t>(std::time(nullptr));}

public:
    static UIDGenerator& getInstance() {
        static UIDGenerator instance;
        return instance;
    }

    uint64_t generateID() {
        uint64_t timestamp = getCurrentTimestamp();
        uint64_t id = (timestamp << TIMESTAMP_BITS) | counter.fetch_add(1);
        return id;
    }

};