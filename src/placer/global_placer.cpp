#include "global_placer.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>

namespace eda {

GlobalPlacer::GlobalPlacer(CircuitModel& circuit)
    : PlacerBase(circuit), rng_(std::random_device{}()) {}

bool GlobalPlacer::run() {
    std::cout << "\n========== " << getName() << " ==========\n";

    Timer timer;
    timer.start();

    // 1. Initialize placement
    std::cout << "Initializing placement...\n";
    initializePlacement();

    // 2. Initialize density grid
    std::cout << "Initializing density grid (" << num_bins_x_ << "x" << num_bins_y_ << ")...\n";
    initDensityGrid();

    // 3. Initialize filler cells
    std::cout << "Initializing filler cells...\n";
    initFillers();

    // Record initial state
    double initial_hpwl = calculateHPWL();
    double initial_overflow = calculateOverflow();
    std::cout << "Initial HPWL: " << initial_hpwl << "\n";
    std::cout << "Initial Overflow: " << initial_overflow * 100.0 << "%\n";

    // 4. Gradient descent optimization
    std::cout << "\nRunning gradient descent optimization...\n";

    std::vector<double> wire_grad_x, wire_grad_y;
    std::vector<double> density_grad_x, density_grad_y;
    std::vector<double> total_grad_x, total_grad_y;

    int iter;
    double current_hpwl = initial_hpwl;
    double current_overflow = initial_overflow;

    for (iter = 0; iter < max_iterations_; ++iter) {
        // Update density
        updateDensity();

        // Compute gradients
        wire_grad_x.clear(); wire_grad_y.clear();
        density_grad_x.clear(); density_grad_y.clear();

        computeWirelengthGradient(wire_grad_x, wire_grad_y);
        computeDensityGradient(density_grad_x, density_grad_y);

        // Combine gradients
        size_t num_movable = circuit_.getNumMovableModules();
        total_grad_x.resize(num_movable);
        total_grad_y.resize(num_movable);

        // Weight for density penalty (increases over iterations)
        double lambda = 0.1 * (1.0 + iter / 50.0);

        for (size_t i = 0; i < num_movable; ++i) {
            total_grad_x[i] = wire_grad_x[i] + lambda * density_grad_x[i];
            total_grad_y[i] = wire_grad_y[i] + lambda * density_grad_y[i];
        }

        // Compute step size
        double step_size = computeStepSize(total_grad_x, total_grad_y);

        // Apply gradient
        applyGradient(total_grad_x, total_grad_y, step_size);

        // Report progress every 20 iterations
        if ((iter + 1) % 20 == 0 || iter == 0) {
            current_hpwl = calculateHPWL();
            current_overflow = calculateOverflow();
            std::cout << "Iter " << std::setw(4) << iter + 1
                      << ": HPWL=" << std::setw(12) << std::fixed << std::setprecision(2) << current_hpwl
                      << " Overflow=" << std::setw(6) << std::setprecision(2) << current_overflow * 100.0 << "%\n";

            reportProgress(iter + 1, current_hpwl, current_overflow);
        }

        // Check convergence
        if (current_overflow <= target_overflow_) {
            std::cout << "Target overflow reached!\n";
            break;
        }
    }

    // Remove fillers
    removeFillers();

    // Calculate final stats
    double final_hpwl = calculateHPWL();
    double final_overflow = calculateOverflow();
    double runtime = timer.elapsed();

    // Update statistics
    updateStats(initial_hpwl, final_hpwl, iter + 1, runtime);
    stats_.initial_overflow = initial_overflow;
    stats_.final_overflow = final_overflow;

    // Print results
    std::cout << "\nFinal HPWL: " << final_hpwl << "\n";
    std::cout << "Final Overflow: " << final_overflow * 100.0 << "%\n";
    std::cout << "Improvement: " << stats_.hpwl_improvement << "%\n";
    std::cout << "Runtime: " << runtime << " seconds\n";
    std::cout << "=====================================\n";

    return true;
}

void GlobalPlacer::initializePlacement() {
    // Random initial placement for movable modules
    std::uniform_real_distribution<double> dist_x(
        circuit_.die_area.x_min, circuit_.die_area.x_max);
    std::uniform_real_distribution<double> dist_y(
        circuit_.die_area.y_min, circuit_.die_area.y_max);

    for (auto& module : circuit_.modules) {
        if (!module->is_fixed && module->type == ModuleType::STANDARD_CELL) {
            module->position.x = dist_x(rng_) - module->width / 2.0;
            module->position.y = dist_y(rng_) - module->height / 2.0;
        }
    }
}

void GlobalPlacer::initDensityGrid() {
    bins_.resize(num_bins_x_ * num_bins_y_);

    double bin_width = circuit_.die_area.width() / num_bins_x_;
    double bin_height = circuit_.die_area.height() / num_bins_y_;

    for (int iy = 0; iy < num_bins_y_; ++iy) {
        for (int ix = 0; ix < num_bins_x_; ++ix) {
            int idx = getBinIndex(ix, iy);
            bins_[idx].ix = ix;
            bins_[idx].iy = iy;
            bins_[idx].x_min = circuit_.die_area.x_min + ix * bin_width;
            bins_[idx].y_min = circuit_.die_area.y_min + iy * bin_height;
            bins_[idx].x_max = bins_[idx].x_min + bin_width;
            bins_[idx].y_max = bins_[idx].y_min + bin_height;
            bins_[idx].area = bin_width * bin_height;
            bins_[idx].target_density = target_density_;
        }
    }
}

void GlobalPlacer::initFillers() {
    // Calculate total movable area
    double total_movable_area = 0.0;
    for (const auto& module : circuit_.modules) {
        if (!module->is_fixed && module->type == ModuleType::STANDARD_CELL) {
            total_movable_area += module->width * module->height;
        }
    }

    // Calculate core area
    double core_area = circuit_.die_area.area();

    // Calculate required filler area
    double target_total_area = core_area * target_density_;
    double fixed_area = 0.0;
    for (const auto& module : circuit_.modules) {
        if (module->is_fixed) {
            fixed_area += module->width * module->height;
        }
    }

    double filler_area_needed = target_total_area - total_movable_area - fixed_area;

    if (filler_area_needed <= 0) return;

    // Create filler cells (simplified - use average cell size)
    double avg_cell_area = total_movable_area / circuit_.getNumMovableModules();
    if (avg_cell_area <= 0) avg_cell_area = 4.0;

    int num_fillers = static_cast<int>(filler_area_needed / avg_cell_area);

    std::uniform_real_distribution<double> dist_x(
        circuit_.die_area.x_min, circuit_.die_area.x_max);
    std::uniform_real_distribution<double> dist_y(
        circuit_.die_area.y_min, circuit_.die_area.y_max);

    double filler_width = std::sqrt(avg_cell_area);
    double filler_height = filler_width;

    for (int i = 0; i < num_fillers; ++i) {
        auto filler = std::make_shared<Module>("filler_" + std::to_string(i));
        filler->type = ModuleType::FILLER;
        filler->is_fixed = false;
        filler->width = filler_width;
        filler->height = filler_height;
        filler->position.x = dist_x(rng_);
        filler->position.y = dist_y(rng_);

        fillers_.push_back(filler);
    }

    std::cout << "  Created " << num_fillers << " filler cells\n";
}

void GlobalPlacer::updateDensity() {
    // Reset bin densities
    for (auto& bin : bins_) {
        bin.node_density = 0.0;
    }

    // Add movable modules
    for (const auto& module : circuit_.modules) {
        if (!module->is_fixed && module->type == ModuleType::STANDARD_CELL) {
            addModuleDensity(module.get());
        }
    }

    // Add fillers
    for (const auto& filler : fillers_) {
        addModuleDensity(filler.get(), 0.5); // Filler has lower weight
    }

    // Add fixed modules
    for (const auto& module : circuit_.modules) {
        if (module->is_fixed) {
            addModuleDensity(module.get());
        }
    }

    // Normalize by bin area
    for (auto& bin : bins_) {
        bin.node_density /= bin.area;
    }
}

void GlobalPlacer::addModuleDensity(const Module* module, double scale) {
    double module_area = module->width * module->height * scale;

    // Find overlapping bins
    int ix_min = static_cast<int>((module->position.x - circuit_.die_area.x_min) /
                                   (circuit_.die_area.width() / num_bins_x_));
    int ix_max = static_cast<int>((module->position.x + module->width - circuit_.die_area.x_min) /
                                   (circuit_.die_area.width() / num_bins_x_));
    int iy_min = static_cast<int>((module->position.y - circuit_.die_area.y_min) /
                                   (circuit_.die_area.height() / num_bins_y_));
    int iy_max = static_cast<int>((module->position.y + module->height - circuit_.die_area.y_min) /
                                   (circuit_.die_area.height() / num_bins_y_));

    ix_min = std::max(0, std::min(ix_min, num_bins_x_ - 1));
    ix_max = std::max(0, std::min(ix_max, num_bins_x_ - 1));
    iy_min = std::max(0, std::min(iy_min, num_bins_y_ - 1));
    iy_max = std::max(0, std::min(iy_max, num_bins_y_ - 1));

    // Distribute area to overlapping bins
    for (int ix = ix_min; ix <= ix_max; ++ix) {
        for (int iy = iy_min; iy <= iy_max; ++iy) {
            int idx = getBinIndex(ix, iy);
            bins_[idx].node_density += module_area;
        }
    }
}

double GlobalPlacer::calculateOverflow() const {
    double total_overflow = 0.0;
    double total_capacity = 0.0;

    for (const auto& bin : bins_) {
        total_overflow += bin.getOverflow() * bin.area;
        total_capacity += bin.area;
    }

    if (total_capacity > 0) {
        return total_overflow / total_capacity;
    }
    return 0.0;
}

void GlobalPlacer::computeWirelengthGradient(std::vector<double>& grad_x,
                                              std::vector<double>& grad_y) {
    size_t num_movable = circuit_.getNumMovableModules();
    grad_x.assign(num_movable, 0.0);
    grad_y.assign(num_movable, 0.0);

    // Build module index map
    std::map<Module*, size_t> module_idx;
    size_t idx = 0;
    for (const auto& module : circuit_.modules) {
        if (!module->is_fixed && module->type == ModuleType::STANDARD_CELL) {
            module_idx[module.get()] = idx++;
        }
    }

    // Compute gradient for each net (simplified - using HPWL gradient)
    for (const auto& net : circuit_.nets) {
        if (net->pins.size() < 2) continue;

        // Find bounding box
        double min_x = net->pins[0]->position.x;
        double max_x = net->pins[0]->position.x;
        double min_y = net->pins[0]->position.y;
        double max_y = net->pins[0]->position.y;

        for (const auto& pin : net->pins) {
            min_x = std::min(min_x, pin->position.x);
            max_x = std::max(max_x, pin->position.x);
            min_y = std::min(min_y, pin->position.y);
            max_y = std::max(max_y, pin->position.y);
        }

        // Compute gradient for each pin
        for (const auto& pin : net->pins) {
            ModulePtr parent = pin->parent_module.lock();
            if (!parent || parent->is_fixed) continue;

            auto it = module_idx.find(parent.get());
            if (it == module_idx.end()) continue;

            size_t m_idx = it->second;

            // Gradient for x
            if (std::abs(pin->position.x - min_x) < 1e-6) {
                grad_x[m_idx] -= 1.0;
            }
            if (std::abs(pin->position.x - max_x) < 1e-6) {
                grad_x[m_idx] += 1.0;
            }

            // Gradient for y
            if (std::abs(pin->position.y - min_y) < 1e-6) {
                grad_y[m_idx] -= 1.0;
            }
            if (std::abs(pin->position.y - max_y) < 1e-6) {
                grad_y[m_idx] += 1.0;
            }
        }
    }
}

void GlobalPlacer::computeDensityGradient(std::vector<double>& grad_x,
                                           std::vector<double>& grad_y) {
    size_t num_movable = circuit_.getNumMovableModules();
    grad_x.assign(num_movable, 0.0);
    grad_y.assign(num_movable, 0.0);

    // Build module index map
    std::map<Module*, size_t> module_idx;
    size_t idx = 0;
    for (const auto& module : circuit_.modules) {
        if (!module->is_fixed && module->type == ModuleType::STANDARD_CELL) {
            module_idx[module.get()] = idx++;
        }
    }

    // Simplified density gradient: push modules away from high-density bins
    for (const auto& module : circuit_.modules) {
        if (module->is_fixed || module->type != ModuleType::STANDARD_CELL) continue;

        auto it = module_idx.find(module.get());
        if (it == module_idx.end()) continue;

        size_t m_idx = it->second;

        Point2D center = module->getCenter();
        auto [ix, iy] = getBinCoord(center.x, center.y);

        if (ix < 0 || ix >= num_bins_x_ || iy < 0 || iy >= num_bins_y_) continue;

        int bin_idx = getBinIndex(ix, iy);
        const auto& bin = bins_[bin_idx];

        if (bin.node_density > bin.target_density) {
            // Find direction to lower density
            double dx = 0, dy = 0;
            int count = 0;

            // Check neighboring bins
            for (int dx_bin = -1; dx_bin <= 1; ++dx_bin) {
                for (int dy_bin = -1; dy_bin <= 1; ++dy_bin) {
                    int nx = ix + dx_bin;
                    int ny = iy + dy_bin;

                    if (nx < 0 || nx >= num_bins_x_ || ny < 0 || ny >= num_bins_y_) continue;

                    int n_idx = getBinIndex(nx, ny);
                    if (bins_[n_idx].node_density < bin.node_density) {
                        dx += dx_bin;
                        dy += dy_bin;
                        count++;
                    }
                }
            }

            if (count > 0) {
                grad_x[m_idx] = dx / count;
                grad_y[m_idx] = dy / count;
            }
        }
    }
}

double GlobalPlacer::computeStepSize(const std::vector<double>& grad_x,
                                      const std::vector<double>& grad_y) {
    // Compute gradient magnitude
    double max_grad = 0.0;
    for (size_t i = 0; i < grad_x.size(); ++i) {
        double grad_mag = std::sqrt(grad_x[i] * grad_x[i] + grad_y[i] * grad_y[i]);
        max_grad = std::max(max_grad, grad_mag);
    }

    if (max_grad < 1e-10) return 0.0;

    // Step size inversely proportional to gradient magnitude
    double die_size = std::max(circuit_.die_area.width(), circuit_.die_area.height());
    return learning_rate_ * die_size / max_grad;
}

void GlobalPlacer::applyGradient(const std::vector<double>& grad_x,
                                  const std::vector<double>& grad_y,
                                  double step_size) {
    size_t idx = 0;
    for (auto& module : circuit_.modules) {
        if (module->is_fixed || module->type != ModuleType::STANDARD_CELL) continue;

        if (idx < grad_x.size()) {
            module->position.x += step_size * grad_x[idx];
            module->position.y += step_size * grad_y[idx];

            // Clip to die area
            module->position.x = std::max(circuit_.die_area.x_min,
                std::min(module->position.x, circuit_.die_area.x_max - module->width));
            module->position.y = std::max(circuit_.die_area.y_min,
                std::min(module->position.y, circuit_.die_area.y_max - module->height));

            idx++;
        }
    }

    // Also move fillers
    for (auto& filler : fillers_) {
        // Random walk for fillers
        std::normal_distribution<double> dist(0.0, step_size * 0.1);
        filler->position.x += dist(rng_);
        filler->position.y += dist(rng_);

        // Clip to die area
        filler->position.x = std::max(circuit_.die_area.x_min,
            std::min(filler->position.x, circuit_.die_area.x_max - filler->width));
        filler->position.y = std::max(circuit_.die_area.y_min,
            std::min(filler->position.y, circuit_.die_area.y_max - filler->height));
    }
}

void GlobalPlacer::removeFillers() {
    fillers_.clear();
}

std::pair<int, int> GlobalPlacer::getBinCoord(double x, double y) const {
    int ix = static_cast<int>((x - circuit_.die_area.x_min) /
                               (circuit_.die_area.width() / num_bins_x_));
    int iy = static_cast<int>((y - circuit_.die_area.y_min) /
                               (circuit_.die_area.height() / num_bins_y_));
    return {ix, iy};
}

} // namespace eda
