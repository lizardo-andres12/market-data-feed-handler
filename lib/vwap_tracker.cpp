#ifndef VWAP_TRACKER_CPP
#define VWAP_TRACKER_CPP

#include <iomanip>
#include <iostream>

#include "lib/vwap_tracker.hpp"


std::optional<const VWAPEntry*> VWAPTracker::getVWAP(std::uint64_t symbol) const {
    auto it = tracker_.find(symbol);
    if (it == tracker_.end()) {
	return std::nullopt;
    }
    return &it->second;
}

void VWAPTracker::upsertVWAP(std::uint64_t symbol, std::uint64_t price, std::uint32_t quantity) {
    auto it = tracker_.find(symbol);
    if (it == tracker_.end()) {
	tracker_[symbol] = {price * quantity, quantity, 1};
	return;
    }
    VWAPEntry& entry = it->second;
    ++entry.totalTrades;
    entry.totalPriceByQuantity += price * quantity;
    entry.totalQuantity += quantity;
    return;
}

void VWAPTracker::showStats() const {
    std::cout << "\n=== VWAP Statistics ===\n";
    std::cout << std::left << std::setw(12) << "Symbol"
              << std::right << std::setw(12) << "VWAP"
              << std::setw(15) << "Total Qty"
              << std::setw(15) << "Trade Count\n";
    std::cout << std::string(54, '-') << '\n';
    
    for (const auto& [symbol, vwap] : tracker_) {
        if (vwap.totalQuantity == 0) continue;
        
        double vwap_price = static_cast<double>(vwap.totalPriceByQuantity) / vwap.totalQuantity / 100.0;
        
        std::cout << std::left << std::setw(12) << symbol
                  << std::right << "$" << std::fixed << std::setprecision(2)
                  << std::setw(10) << vwap_price
                  << std::setw(15) << vwap.totalQuantity
                  << std::setw(15) << vwap.totalTrades << '\n';
    }
}


#endif

