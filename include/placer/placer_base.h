#ifndef EDA_PLACER_PLACER_BASE_H
#define EDA_PLACER_PLACER_BASE_H

#include "circuit_model.h"
#include <string>
#include <functional>
#include <chrono>

namespace eda {

/**
 * @brief Placement algorithm types
 */
enum class PlacerType {
    INITIAL_KRAFTWERK2A,   // Kraftwerk2A initial placement
    GLOBAL_EPLACE_MS,      // ePlace-MS global placement
    CUSTOM                 // Custom/user-defined placer
};

/**
 * @brief Placement statistics
 */
struct PlacementStats {
    double initial_hpwl = 0.0;
    double final_hpwl = 0.0;
    double hpwl_improvement = 0.0;
    double initial_overflow = 0.0;
    double final_overflow = 0.0;
    int iterations = 0;
    double runtime_seconds = 0.0;

    void print() const;
};

/**
 * @brief Progress callback function type
 */
using ProgressCallback = std::function<void(int iteration, double hpwl, double overflow)>;

/**
 * @brief Base class for all placement algorithms
 */
class PlacerBase {
public:
    explicit PlacerBase(CircuitModel& circuit);
    virtual ~PlacerBase() = default;

    /**
     * @brief Run the placement algorithm
     * @return true if placement succeeded
     */
    virtual bool run() = 0;

    /**
     * @brief Get the placer type
     */
    virtual PlacerType getType() const = 0;

    /**
     * @brief Get the placer name
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Set progress callback
     */
    void setProgressCallback(ProgressCallback callback);

    /**
     * @brief Get placement statistics
     */
    const PlacementStats& getStats() const { return stats_; }

    /**
     * @brief Get last error message
     */
    std::string getLastError() const { return last_error_; }

    /**
     * @brief Calculate current total HPWL
     */
    double calculateHPWL() const;

    /**
     * @brief Calculate current density overflow
     */
    virtual double calculateOverflow() const;

protected:
    CircuitModel& circuit_;
    PlacementStats stats_;
    std::string last_error_;
    ProgressCallback progress_callback_;

    // Helper methods
    void reportProgress(int iteration, double hpwl, double overflow);
    void updateStats(double initial_hpwl, double final_hpwl,
                     int iterations, double runtime_seconds);

    // Timer helper
    class Timer {
    public:
        void start();
        double elapsed() const;  // in seconds
    private:
        std::chrono::high_resolution_clock::time_point start_time_;
    };
};

/**
 * @brief Placer factory
 */
class PlacerFactory {
public:
    static std::unique_ptr<PlacerBase> create(PlacerType type, CircuitModel& circuit);
    static std::unique_ptr<PlacerBase> create(const std::string& name, CircuitModel& circuit);
};

} // namespace eda

#endif // EDA_PLACER_PLACER_BASE_H
