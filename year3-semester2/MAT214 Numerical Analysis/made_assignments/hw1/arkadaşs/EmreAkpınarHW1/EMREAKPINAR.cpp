// main.cpp
// MATH 214 - Assignment 1 driver.
// Runs all three test problems with each method and writes:
//   - results.csv  : summary table (one row per run)
//   - iterates_pX_<label>.csv : full sequence x_k for each run (for plotting)

#include "solvers.hpp"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <cmath>

using nm::SolverResult;

// High-precision reference roots for error plots
constexpr double X_STAR_P1 = 1.5213797068045676;  // root of x^3 - x - 2
constexpr double X_STAR_P2 = 0.7390851332151607;  // root of cos(x) - x  (Dottie)
constexpr double X_STAR_P3 = 0.0;                 // triple root of x^3

// ---------- I/O helpers ---------------------------------------------------

static std::string sanitize(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (std::isalnum(static_cast<unsigned char>(c))) out.push_back(c);
        else if (c == '.' || c == '-' || c == '+') out.push_back(c);
        else out.push_back('_');
    }
    return out;
}

static void write_iterates(const std::string& dir, const std::string& tag,
                           const SolverResult& R, double x_star)
{
    std::string path = dir + "/iterates_" + sanitize(tag) + ".csv";
    std::ofstream f(path);
    f << "k,x_k,err_abs\n";
    f << std::setprecision(17);
    for (size_t k = 0; k < R.iterates.size(); ++k) {
        double xk = R.iterates[k];
        double e  = std::isfinite(xk) ? std::fabs(xk - x_star)
                                      : std::numeric_limits<double>::infinity();
        f << k << "," << xk << "," << e << "\n";
    }
}

static void append_summary(std::ofstream& csv, const std::string& problem,
                           const SolverResult& R)
{
    csv << "\"" << problem << "\","
        << "\"" << R.method << "\","
        << "\"" << R.label  << "\","
        << R.iterations << ",";
    if (std::isfinite(R.root))     csv << std::setprecision(15) << R.root;     else csv << "inf";
    csv << ",";
    if (std::isfinite(R.residual)) csv << std::setprecision(6)  << R.residual; else csv << "inf";
    csv << "," << (R.converged ? "true" : "false")
        << ",\"" << R.reason << "\"\n";
}

static void print_run(const SolverResult& R) {
    std::cout << std::left << std::setw(48) << R.label
              << " iters=" << std::setw(4) << R.iterations
              << "  root=" << std::setprecision(12) << std::setw(20) << R.root
              << "  res="  << std::scientific << std::setprecision(3) << R.residual
              << std::defaultfloat
              << "  [" << R.reason << "]\n";
}

// ---------- Problem definitions ------------------------------------------

static void problem1(std::ofstream& csv, const std::string& outdir) {
    auto f  = [](double x) { return x*x*x - x - 2.0; };
    auto fp = [](double x) { return 3.0*x*x - 1.0;   };

    std::cout << "\n=== Problem 1: f(x) = x^3 - x - 2  (x* = 1.521379...) ===\n";

    // Bisection on [1, 2]
    {
        auto R = nm::bisection(f, 1.0, 2.0);
        R.label = "Bisection on [1,2]";
        print_run(R);
        write_iterates(outdir, "p1_bisection", R, X_STAR_P1);
        append_summary(csv, "P1: x^3-x-2", R);
    }
    // Fixed-Point form A:  g(x) = (x+2)^(1/3),  |g'(x*)| < 1  -> converges
    {
        auto g = [](double x) { return std::cbrt(x + 2.0); };
        auto R = nm::fixed_point(g, f, 1.5);
        R.label = "Fixed-Point g(x)=(x+2)^(1/3), x0=1.5";
        print_run(R);
        write_iterates(outdir, "p1_fp_good", R, X_STAR_P1);
        append_summary(csv, "P1: x^3-x-2", R);
    }
    // Fixed-Point form B:  g(x) = x^3 - 2,  |g'(x*)| ~ 6.94 > 1  -> diverges
    {
        auto g = [](double x) { return x*x*x - 2.0; };
        auto R = nm::fixed_point(g, f, 1.5);
        R.label = "Fixed-Point g(x)=x^3-2, x0=1.5";
        print_run(R);
        write_iterates(outdir, "p1_fp_bad", R, X_STAR_P1);
        append_summary(csv, "P1: x^3-x-2", R);
    }
    // Newton from x0 = 1.5
    {
        auto R = nm::newton(f, fp, 1.5);
        R.label = "Newton, x0=1.5";
        print_run(R);
        write_iterates(outdir, "p1_newton", R, X_STAR_P1);
        append_summary(csv, "P1: x^3-x-2", R);
    }
}

static void problem2(std::ofstream& csv, const std::string& outdir) {
    auto f  = [](double x) { return std::cos(x) - x;          };
    auto fp = [](double x) { return -std::sin(x) - 1.0;       };

    std::cout << "\n=== Problem 2: f(x) = cos(x) - x  (x* = 0.739085...) ===\n";

    // Bisection on [0, 1] for reference
    {
        auto R = nm::bisection(f, 0.0, 1.0);
        R.label = "Bisection on [0,1]";
        print_run(R);
        write_iterates(outdir, "p2_bisection", R, X_STAR_P2);
        append_summary(csv, "P2: cos(x)-x", R);
    }
    // Newton from a range of starting points
    for (double x0 : {0.0, 0.5, 1.0, 5.0, 50.0}) {
        auto R = nm::newton(f, fp, x0);
        R.label = "Newton, x0=" + std::to_string(x0);
        print_run(R);
        write_iterates(outdir, "p2_newton_x0_" + std::to_string(x0), R, X_STAR_P2);
        append_summary(csv, "P2: cos(x)-x", R);
    }
}

static void problem3(std::ofstream& csv, const std::string& outdir) {
    auto f  = [](double x) { return x*x*x;     };
    auto fp = [](double x) { return 3.0*x*x;   };

    std::cout << "\n=== Problem 3: f(x) = x^3 (triple root at x* = 0) ===\n";

    // Bisection on [-1, 1]
    {
        auto R = nm::bisection(f, -1.0, 1.0);
        R.label = "Bisection on [-1,1]";
        print_run(R);
        write_iterates(outdir, "p3_bisection", R, X_STAR_P3);
        append_summary(csv, "P3: x^3", R);
    }
    // Standard Newton from x0 = 1.0 -> linear, slow
    {
        auto R = nm::newton(f, fp, 1.0);
        R.label = "Newton (standard), x0=1.0";
        print_run(R);
        write_iterates(outdir, "p3_newton_std_x0_1", R, X_STAR_P3);
        append_summary(csv, "P3: x^3", R);
    }
    // Standard Newton from x0 = 0.5 -> still linear
    {
        auto R = nm::newton(f, fp, 0.5);
        R.label = "Newton (standard), x0=0.5";
        print_run(R);
        write_iterates(outdir, "p3_newton_std_x0_05", R, X_STAR_P3);
        append_summary(csv, "P3: x^3", R);
    }
    // Modified Newton with multiplicity m=3 -> quadratic
    {
        auto R = nm::newton_modified(f, fp, 1.0, 3);
        R.label = "Newton modified m=3, x0=1.0";
        print_run(R);
        write_iterates(outdir, "p3_newton_mod_x0_1", R, X_STAR_P3);
        append_summary(csv, "P3: x^3", R);
    }
}

// ---------- main ---------------------------------------------------------

int main(int argc, char** argv) {
    std::string outdir = (argc >= 2) ? argv[1] : "data";

    std::ofstream csv(outdir + "/results.csv");
    csv << "problem,method,setup,iterations,root,residual,converged,reason\n";

    problem1(csv, outdir);
    problem2(csv, outdir);
    problem3(csv, outdir);

    std::cout << "\nWrote summary -> " << outdir << "/results.csv\n";
    return 0;
}
