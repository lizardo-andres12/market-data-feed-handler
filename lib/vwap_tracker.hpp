#ifndef VWAP_TRACKER_HPP
#define VWAP_TRACKER_HPP

#include <cstdint>
#include <optional>
#include <unordered_map>


struct VWAPEntry {
    std::uint64_t totalPriceByQuantity;
    std::uint64_t totalQuantity;
    std::uint32_t totalTrades;
};

class VWAPTracker {
    std::unordered_map<std::uint64_t, VWAPEntry> tracker_;
public:
    VWAPTracker() = default;

    std::optional<const VWAPEntry*> getVWAP(std::uint64_t symbol) const;

    void upsertVWAP(std::uint64_t symbol, std::uint64_t price, std::uint32_t quantity);

    void showStats() const;

    VWAPTracker(const VWAPTracker& vt) = delete;
    VWAPTracker(VWAPTracker&& vt) = delete;
    void operator=(const VWAPTracker& vt) = delete;
    void operator=(VWAPTracker&& vt) = delete;
};


#endif

