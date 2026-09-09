// solvers.hpp
// MATH 214 - Assignment 1
// Modular root-finding solvers: Bisection, Fixed-Point, Newton.
//
// Each solver returns a SolverResult containing the final root,
// residual |f(root)|, iteration count, and the full sequence of iterates.
// Stopping criteria: |x_{k+1} - x_k| < eps  OR  |f(x_k)| < eps.
// max_iter caps the loop to prevent runaway iterations.

#ifndef SOLVERS_HPP
#define SOLVERS_HPP

#include <vector>
#include <string>
#include <functional>
#include <cmath>
#include <stdexcept>
#include <limits>

namespace nm {

constexpr double EPS      = 1e-6;
constexpr int    MAX_ITER = 100;

struct SolverResult {
    std::string method;       // "Bisection" / "Fixed-Point" / "Newton"
    std::string label;        // human-readable setup label
    double      root;         // final approximation
    double      residual;     // |f(root)|
    int         iterations;   // total iterations performed
    std::vector<double> iterates; // full sequence x_0, x_1, ..., x_n
    bool        converged;    // true if a stopping criterion was met
    std::string reason;       // why the loop stopped
};

// ---------------------------------------------------------------------------
// Bisection on [a, b] with f(a)*f(b) < 0.
// ---------------------------------------------------------------------------
inline SolverResult bisection(std::function<double(double)> f,
                              double a, double b,
                              double eps = EPS, int max_iter = MAX_ITER)
{
    SolverResult R;
    R.method = "Bisection";
    R.converged = false;
    R.reason = "max_iter reached";

    double fa = f(a), fb = f(b);
    if (fa * fb > 0.0)
        throw std::invalid_argument("Bisection requires f(a)*f(b) < 0");

    double x_prev = a;
    double c = 0.0, fc = 0.0;

    for (int k = 1; k <= max_iter; ++k) {
        c  = 0.5 * (a + b);
        fc = f(c);
        R.iterates.push_back(c);

        // step size: distance from previous midpoint (or interval half-width on iter 1)
        double step = (k > 1) ? std::fabs(c - x_prev) : 0.5 * std::fabs(b - a);

        if (std::fabs(fc) < eps) {
            R.converged = true;
            R.reason = "|f(x_k)| < eps";
            break;
        }
        if (k > 1 && step < eps) {
            R.converged = true;
            R.reason = "|x_{k+1} - x_k| < eps";
            break;
        }

        // narrow the bracket
        if (fa * fc < 0.0) { b = c; fb = fc; }
        else               { a = c; fa = fc; }
        x_prev = c;
    }

    R.root       = R.iterates.back();
    R.residual   = std::fabs(f(R.root));
    R.iterations = static_cast<int>(R.iterates.size());
    return R;
}

// ---------------------------------------------------------------------------
// Fixed-point iteration: x_{k+1} = g(x_k).
// Residual is computed against the original f (passed in for diagnostics).
// ---------------------------------------------------------------------------
inline SolverResult fixed_point(std::function<double(double)> g,
                                std::function<double(double)> f,
                                double x0,
                                double eps = EPS, int max_iter = MAX_ITER)
{
    SolverResult R;
    R.method = "Fixed-Point";
    R.converged = false;
    R.reason = "max_iter reached";
    R.iterates.push_back(x0);

    double x = x0;
    for (int k = 1; k <= max_iter; ++k) {
        double x_new = g(x);

        if (!std::isfinite(x_new)) {
            R.reason = "diverged to non-finite";
            R.iterates.push_back(x_new);
            x = x_new;
            break;
        }

        R.iterates.push_back(x_new);

        if (std::fabs(x_new - x) < eps) {
            R.converged = true;
            R.reason = "|x_{k+1} - x_k| < eps";
            x = x_new;
            break;
        }
        x = x_new;
    }

    R.root       = x;
    R.residual   = std::isfinite(x) ? std::fabs(f(x))
                                    : std::numeric_limits<double>::infinity();
    R.iterations = static_cast<int>(R.iterates.size()) - 1;
    return R;
}

// ---------------------------------------------------------------------------
// Newton's method: x_{k+1} = x_k - f(x_k)/f'(x_k).
// ---------------------------------------------------------------------------
inline SolverResult newton(std::function<double(double)> f,
                           std::function<double(double)> fprime,
                           double x0,
                           double eps = EPS, int max_iter = MAX_ITER)
{
    SolverResult R;
    R.method = "Newton";
    R.converged = false;
    R.reason = "max_iter reached";
    R.iterates.push_back(x0);

    double x = x0;
    for (int k = 1; k <= max_iter; ++k) {
        double fx = f(x);
        if (std::fabs(fx) < eps) {
            R.converged = true;
            R.reason = "|f(x_k)| < eps";
            break;
        }
        double fpx = fprime(x);
        if (fpx == 0.0) {
            R.reason = "zero derivative encountered";
            break;
        }
        double x_new = x - fx / fpx;

        if (!std::isfinite(x_new)) {
            R.reason = "diverged to non-finite";
            R.iterates.push_back(x_new);
            x = x_new;
            break;
        }
        R.iterates.push_back(x_new);

        if (std::fabs(x_new - x) < eps) {
            R.converged = true;
            R.reason = "|x_{k+1} - x_k| < eps";
            x = x_new;
            break;
        }
        x = x_new;
    }

    R.root       = x;
    R.residual   = std::isfinite(x) ? std::fabs(f(x))
                                    : std::numeric_limits<double>::infinity();
    R.iterations = static_cast<int>(R.iterates.size()) - 1;
    return R;
}

// ---------------------------------------------------------------------------
// Modified Newton for a known multiplicity m:  x_{k+1} = x_k - m * f / f'
// Restores quadratic convergence at multiple roots.
// ---------------------------------------------------------------------------
inline SolverResult newton_modified(std::function<double(double)> f,
                                    std::function<double(double)> fprime,
                                    double x0, int m,
                                    double eps = EPS, int max_iter = MAX_ITER)
{
    SolverResult R;
    R.method = "Newton (modified, m=" + std::to_string(m) + ")";
    R.converged = false;
    R.reason = "max_iter reached";
    R.iterates.push_back(x0);

    double x = x0;
    for (int k = 1; k <= max_iter; ++k) {
        double fx = f(x);
        if (std::fabs(fx) < eps) {
            R.converged = true;
            R.reason = "|f(x_k)| < eps";
            break;
        }
        double fpx = fprime(x);
        if (fpx == 0.0) {
            R.reason = "zero derivative encountered";
            break;
        }
        double x_new = x - m * fx / fpx;
        R.iterates.push_back(x_new);
        if (std::fabs(x_new - x) < eps) {
            R.converged = true;
            R.reason = "|x_{k+1} - x_k| < eps";
            x = x_new;
            break;
        }
        x = x_new;
    }

    R.root       = x;
    R.residual   = std::fabs(f(x));
    R.iterations = static_cast<int>(R.iterates.size()) - 1;
    return R;
}

} // namespace nm

#endif // SOLVERS_HPP
