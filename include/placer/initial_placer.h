#ifndef EDA_PLACER_INITIAL_PLACER_H
#define EDA_PLACER_INITIAL_PLACER_H

#include "placer_base.h"
#include <Eigen/Sparse>
#include <vector>
#include <map>

namespace eda {

/**
 * @brief Kraftwerk2A Initial Placer
 *
 * Implements quadratic placement using B2B (Bound2Bound) net model
 * and BiCGSTAB iterative solver.
 */
class InitialPlacer : public PlacerBase {
public:
    explicit InitialPlacer(CircuitModel& circuit);
    ~InitialPlacer() override = default;

    bool run() override;
    PlacerType getType() const override { return PlacerType::INITIAL_KRAFTWERK2A; }
    std::string getName() const override { return "Kraftwerk2A Initial Placer"; }

    // Parameters
    void setMaxIterations(int max_iter) { max_iterations_ = max_iter; }
    void setTargetError(double error) { target_error_ = error; }

private:
    // Eigen typedefs
    using SpMat = Eigen::SparseMatrix<double, Eigen::RowMajor>;
    using Triplet = Eigen::Triplet<double>;

    // Parameters
    int max_iterations_ = 100;
    double target_error_ = 1e-4;

    // Data structures
    std::map<Module*, int> module_to_index_;
    std::vector<Module*> index_to_module_;
    int movable_count_ = 0;

    // Linear system
    SpMat A_;                    // Laplacian matrix
    Eigen::VectorXd b_x_, b_y_;  // RHS vectors
    Eigen::VectorXd x_, y_;      // Solution vectors

    // Methods
    bool prepareData();
    void buildLaplacianMatrix();
    void initializePositions();
    bool solveDirection(Eigen::VectorXd& solution, const Eigen::VectorXd& rhs);
    void updateModulePositions();
    void clipToCoreRegion();
};

} // namespace eda

#endif // EDA_PLACER_INITIAL_PLACER_H
