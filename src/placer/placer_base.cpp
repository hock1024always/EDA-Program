#include "placer_base.h"
#include <iostream>
#include <iomanip>

namespace eda {

// PlacementStats implementation
void PlacementStats::print() const {
    std::cout << "\n========== Placement Statistics ==========\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Initial HPWL:    " << initial_hpwl << "\n";
    std::cout << "Final HPWL:      " << final_hpwl << "\n";
    std::cout << "Improvement:     " << hpwl_improvement << "%\n";
    std::cout << "Initial Overflow:" << initial_overflow * 100.0 << "%\n";
    std::cout << "Final Overflow:  " << final_overflow * 100.0 << "%\n";
    std::cout << "Iterations:      " << iterations << "\n";
    std::cout << "Runtime:         " << runtime_seconds << " seconds\n";
    std::cout << "==========================================\n";
}

// PlacerBase implementation
PlacerBase::PlacerBase(CircuitModel& circuit) : circuit_(circuit) {}

void PlacerBase::setProgressCallback(ProgressCallback callback) {
    progress_callback_ = callback;
}

double PlacerBase::calculateHPWL() const {
    return circuit_.calcTotalHPWL();
}

double PlacerBase::calculateOverflow() const {
    // Default implementation - derived classes may override
    return 0.0;
}

void PlacerBase::reportProgress(int iteration, double hpwl, double overflow) {
    if (progress_callback_) {
        progress_callback_(iteration, hpwl, overflow);
    }
}

void PlacerBase::updateStats(double initial_hpwl, double final_hpwl,
                              int iterations, double runtime_seconds) {
    stats_.initial_hpwl = initial_hpwl;
    stats_.final_hpwl = final_hpwl;
    stats_.hpwl_improvement = (initial_hpwl > 0)
        ? ((initial_hpwl - final_hpwl) / initial_hpwl * 100.0)
        : 0.0;
    stats_.iterations = iterations;
    stats_.runtime_seconds = runtime_seconds;
}

// Timer implementation
void PlacerBase::Timer::start() {
    start_time_ = std::chrono::high_resolution_clock::now();
}

double PlacerBase::Timer::elapsed() const {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time_);
    return duration.count() / 1000.0;
}

} // namespace eda
