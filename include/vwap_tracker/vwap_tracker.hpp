#pragma once

#include "messages.hpp"
#include <cstdint>
#include <optional>
#include <unordered_map>


/**
 * @brief A class for storing symbols mapped to executed trade metadata useful for calculating VWAP.
 * Supports insertion of a symbol/entry pair and updating the entry for an existing symbol. This
 * data structure cannot and should not be moved or copied.
 */
class VWAPTracker {
public:
    /**
     * @brief The collection of relevant data to store in the VWAPTracker for a given symbol.
     */
    struct VWAPEntry {
	std::uint64_t updatedAt;
	std::uint64_t totalPriceByQuantity;
	std::uint64_t totalQuantity;
	std::uint32_t totalTrades;
    };

    /**
     * @brief Default constructor for OrderBook that instantiates all member variables.
     */
    VWAPTracker() = default;

    /**
     * @brief Attempts to retrieve the VWAP metadata object associated with the input symbol.
     *
     * @param key The symbol of the instrument stored as the key.
     * @return An optional containing a const pointer to the `VWAPEntry` associated with `key` if it exists.
     */
    std::optional<const VWAPEntry*> getVWAP(std::uint64_t symbol) const;

    /**
     * @brief Creates a new mapping of symbol to VWAP data using the provided `TradeMessage`.
     *
     * @param key The symbol to update or insert.
     * @param msg The data of the incoming trade.
     */
    void upsertVWAP(const std::uint64_t symbol, const TradeMessage& msg);

    std::size_t size() const {
	return tracker_.size();
    }

    /**
     * @brief Displays stored symbols and their metadata.
     */
    void showStats() const;

    VWAPTracker(const VWAPTracker& vt) = delete;
    VWAPTracker(VWAPTracker&& vt) = delete;
    void operator=(const VWAPTracker& vt) = delete;
    void operator=(VWAPTracker&& vt) = delete;

private:
    std::unordered_map<std::uint64_t, VWAPEntry> tracker_;
};

