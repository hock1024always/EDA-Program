#include <cmath>
#include <vector>
#include <algorithm>

namespace eda {

// Mathematical utility functions

/**
 * @brief Solve sparse linear system using Conjugate Gradient
 */
bool solveCG(const std::vector<std::vector<double>>& A,
             const std::vector<double>& b,
             std::vector<double>& x,
             double tolerance = 1e-6,
             int max_iterations = 1000) {
    int n = b.size();
    x.resize(n, 0.0);

    std::vector<double> r = b;
    std::vector<double> p = r;
    double rs_old = 0.0;

    for (int i = 0; i < n; ++i) {
        rs_old += r[i] * r[i];
    }

    for (int iter = 0; iter < max_iterations; ++iter) {
        std::vector<double> Ap(n, 0.0);
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                Ap[i] += A[i][j] * p[j];
            }
        }

        double pAp = 0.0;
        for (int i = 0; i < n; ++i) {
            pAp += p[i] * Ap[i];
        }

        if (std::abs(pAp) < 1e-10) break;

        double alpha = rs_old / pAp;

        for (int i = 0; i < n; ++i) {
            x[i] += alpha * p[i];
            r[i] -= alpha * Ap[i];
        }

        double rs_new = 0.0;
        for (int i = 0; i < n; ++i) {
            rs_new += r[i] * r[i];
        }

        if (std::sqrt(rs_new) < tolerance) break;

        for (int i = 0; i < n; ++i) {
            p[i] = r[i] + (rs_new / rs_old) * p[i];
        }

        rs_old = rs_new;
    }

    return true;
}

/**
 * @brief Clamp value to range [min, max]
 */
template<typename T>
T clamp(T value, T min_val, T max_val) {
    return std::max(min_val, std::min(value, max_val));
}

/**
 * @brief Linear interpolation
 */
double lerp(double a, double b, double t) {
    return a + t * (b - a);
}

} // namespace eda
