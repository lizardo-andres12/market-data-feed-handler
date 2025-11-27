#include <cstring>
#include <iomanip>
#include <iostream>

#include "vwap_tracker/vwap_tracker.hpp"
#include "messages.hpp"


auto VWAPTracker::getVWAP(std::uint64_t symbol) const -> std::optional<const VWAPEntry*> {
    auto it = tracker_.find(symbol);
    if (it == tracker_.end()) {
	return std::nullopt;
    }
    return &it->second;
}

void VWAPTracker::upsertVWAP(std::uint64_t symbol, const TradeMessage& msg) {
    auto it = tracker_.find(symbol);
    if (it == tracker_.end()) {
	tracker_[symbol] = {msg.price * msg.quantity, msg.quantity, 1};
	return;
    }
    VWAPEntry& entry = it->second;
    ++entry.totalTrades;
    entry.totalPriceByQuantity += msg.price * msg.quantity;
    entry.totalQuantity += msg.quantity;
    return;
}

void VWAPTracker::showStats() const {
    std::cout << "\n=== VWAP Statistics ===\n";
    std::cout << std::left << std::setw(12) << "Symbol"
              << std::right << std::setw(12) << "VWAP"
              << std::setw(15) << "Total Qty"
              << std::setw(15) << "Trade Count\n";
    std::cout << std::string(54, '-') << '\n';
    
    char symStr[8] = {0};
    for (const auto& [symbol, vwap] : tracker_) {
        if (vwap.totalQuantity == 0) continue;
        
	std::memcpy(symStr, &symbol, sizeof(symbol));
        double vwap_price = static_cast<double>(vwap.totalPriceByQuantity) / vwap.totalQuantity / 100.0;
        
        std::cout << std::left << std::setw(12) << symStr
                  << std::right << "$" << std::fixed << std::setprecision(2)
                  << std::setw(10) << vwap_price
                  << std::setw(15) << vwap.totalQuantity
                  << std::setw(15) << vwap.totalTrades << '\n';
    }
}

