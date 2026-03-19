#include "initial_placer.h"
#include <Eigen/IterativeLinearSolvers>
#include <iostream>
#include <iomanip>
#include <cmath>

namespace eda {

InitialPlacer::InitialPlacer(CircuitModel& circuit)
    : PlacerBase(circuit) {}

bool InitialPlacer::run() {
    std::cout << "\n========== " << getName() << " ==========\n";

    Timer timer;
    timer.start();

    // 1. Prepare data
    std::cout << "Preparing data...\n";
    if (!prepareData()) {
        last_error_ = "Data preparation failed";
        return false;
    }

    // 2. Build Laplacian matrix
    std::cout << "Building Laplacian matrix...\n";
    buildLaplacianMatrix();

    // 3. Initialize positions
    std::cout << "Initializing positions...\n";
    initializePositions();

    // Record initial HPWL
    double initial_hpwl = calculateHPWL();
    std::cout << "Initial HPWL: " << initial_hpwl << "\n";

    // 4. Solve X direction
    std::cout << "Solving X direction...\n";
    if (!solveDirection(x_, b_x_)) {
        last_error_ = "X direction solve failed";
        return false;
    }

    // 5. Solve Y direction
    std::cout << "Solving Y direction...\n";
    if (!solveDirection(y_, b_y_)) {
        last_error_ = "Y direction solve failed";
        return false;
    }

    // 6. Update module positions
    updateModulePositions();

    // Calculate final HPWL
    double final_hpwl = calculateHPWL();
    double runtime = timer.elapsed();

    // Update statistics
    updateStats(initial_hpwl, final_hpwl, max_iterations_, runtime);
    stats_.initial_overflow = calculateOverflow();
    stats_.final_overflow = stats_.initial_overflow;

    // Print results
    std::cout << "Final HPWL: " << final_hpwl << "\n";
    std::cout << "Improvement: " << stats_.hpwl_improvement << "%\n";
    std::cout << "Runtime: " << runtime << " seconds\n";
    std::cout << "=====================================\n";

    return true;
}

bool InitialPlacer::prepareData() {
    // Count movable modules
    movable_count_ = 0;
    module_to_index_.clear();
    index_to_module_.clear();

    for (const auto& module : circuit_.modules) {
        if (!module->is_fixed && module->type == ModuleType::STANDARD_CELL) {
            module_to_index_[module.get()] = movable_count_;
            index_to_module_.push_back(module.get());
            movable_count_++;
        }
    }

    std::cout << "  Movable modules: " << movable_count_ << "\n";
    std::cout << "  Fixed modules: " << circuit_.getNumFixedModules() << "\n";
    std::cout << "  Nets: " << circuit_.getNumNets() << "\n";

    if (movable_count_ == 0) {
        last_error_ = "No movable modules found";
        return false;
    }

    // Initialize vectors
    x_ = Eigen::VectorXd::Zero(movable_count_);
    y_ = Eigen::VectorXd::Zero(movable_count_);
    b_x_ = Eigen::VectorXd::Zero(movable_count_);
    b_y_ = Eigen::VectorXd::Zero(movable_count_);

    return true;
}

void InitialPlacer::buildLaplacianMatrix() {
    std::vector<Triplet> triplet_list;

    // Iterate over all nets
    for (const auto& net : circuit_.nets) {
        int degree = net->pins.size();
        if (degree < 2) continue;

        // Weight using B2B model: w = 1 / (degree - 1)
        double weight = 1.0 / (degree - 1);

        // Iterate over all pin pairs in the net
        for (size_t i = 0; i < net->pins.size(); i++) {
            Pin* pin_i = net->pins[i].get();
            Module* module_i = pin_i->parent_module.lock().get();
            if (!module_i) continue;

            for (size_t j = i + 1; j < net->pins.size(); j++) {
                Pin* pin_j = net->pins[j].get();
                Module* module_j = pin_j->parent_module.lock().get();
                if (!module_j) continue;

                // Check if both are movable
                bool i_movable = (!module_i->is_fixed &&
                                 module_i->type == ModuleType::STANDARD_CELL);
                bool j_movable = (!module_j->is_fixed &&
                                 module_j->type == ModuleType::STANDARD_CELL);

                if (i_movable && j_movable) {
                    // Both movable
                    int idx_i = module_to_index_[module_i];
                    int idx_j = module_to_index_[module_j];

                    // Diagonal elements
                    triplet_list.emplace_back(idx_i, idx_i, weight);
                    triplet_list.emplace_back(idx_j, idx_j, weight);

                    // Off-diagonal elements
                    triplet_list.emplace_back(idx_i, idx_j, -weight);
                    triplet_list.emplace_back(idx_j, idx_i, -weight);
                }
                else if (i_movable) {
                    // i movable, j fixed
                    int idx_i = module_to_index_[module_i];
                    triplet_list.emplace_back(idx_i, idx_i, weight);

                    // Add to RHS
                    Point2D pos_j = module_j->getCenter();
                    b_x_(idx_i) += weight * (pos_j.x + pin_j->offset.x);
                    b_y_(idx_i) += weight * (pos_j.y + pin_j->offset.y);
                }
                else if (j_movable) {
                    // j movable, i fixed
                    int idx_j = module_to_index_[module_j];
                    triplet_list.emplace_back(idx_j, idx_j, weight);

                    // Add to RHS
                    Point2D pos_i = module_i->getCenter();
                    b_x_(idx_j) += weight * (pos_i.x + pin_i->offset.x);
                    b_y_(idx_j) += weight * (pos_i.y + pin_i->offset.y);
                }
            }
        }
    }

    // Build sparse matrix
    A_.resize(movable_count_, movable_count_);
    A_.setFromTriplets(triplet_list.begin(), triplet_list.end());

    std::cout << "  Matrix non-zeros: " << A_.nonZeros() << "\n";
    std::cout << "  Matrix density: "
              << (double)A_.nonZeros() / (movable_count_ * movable_count_) * 100.0
              << "%\n";
}

void InitialPlacer::initializePositions() {
    // Initialize all movable modules to center of die area
    double center_x = (circuit_.die_area.x_min + circuit_.die_area.x_max) / 2.0;
    double center_y = (circuit_.die_area.y_min + circuit_.die_area.y_max) / 2.0;

    for (int i = 0; i < movable_count_; i++) {
        x_(i) = center_x;
        y_(i) = center_y;
    }
}

bool InitialPlacer::solveDirection(Eigen::VectorXd& solution,
                                    const Eigen::VectorXd& rhs) {
    // Use BiCGSTAB solver
    Eigen::BiCGSTAB<SpMat> solver;
    solver.setMaxIterations(max_iterations_);
    solver.setTolerance(target_error_);

    solver.compute(A_);
    if (solver.info() != Eigen::Success) {
        last_error_ = "Matrix decomposition failed";
        return false;
    }

    solution = solver.solveWithGuess(rhs, solution);

    if (solver.info() != Eigen::Success) {
        last_error_ = "Solve failed";
        return false;
    }

    std::cout << "  Iterations: " << solver.iterations() << "\n";
    std::cout << "  Estimated error: " << solver.error() << "\n";

    return true;
}

void InitialPlacer::updateModulePositions() {
    for (int i = 0; i < movable_count_; i++) {
        Module* module = index_to_module_[i];
        module->setCenter(Point2D(x_(i), y_(i)));
    }

    clipToCoreRegion();
}

void InitialPlacer::clipToCoreRegion() {
    for (int i = 0; i < movable_count_; i++) {
        Module* module = index_to_module_[i];

        double half_w = module->width / 2.0;
        double half_h = module->height / 2.0;

        // X direction clipping
        if (module->position.x < circuit_.die_area.x_min) {
            module->position.x = circuit_.die_area.x_min;
        }
        if (module->position.x + module->width > circuit_.die_area.x_max) {
            module->position.x = circuit_.die_area.x_max - module->width;
        }

        // Y direction clipping
        if (module->position.y < circuit_.die_area.y_min) {
            module->position.y = circuit_.die_area.y_min;
        }
        if (module->position.y + module->height > circuit_.die_area.y_max) {
            module->position.y = circuit_.die_area.y_max - module->height;
        }
    }
}

} // namespace eda
