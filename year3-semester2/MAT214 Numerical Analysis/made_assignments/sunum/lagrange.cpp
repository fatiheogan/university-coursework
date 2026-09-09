// Lagrange Interpolation - Numerical Analysis Project
// Implements Lagrange polynomial interpolation for three functions:
//   f1(x) = cos(x)
//   f2(x) = 1 / (1 + x^2)        (Runge function)
//   f3(x) = ln(x) * sqrt(x)
//
// For each function we use 4 equally-spaced interpolation nodes,
// build P(x) by Lagrange's formula, and compare P(x) against f(x)
// at intermediate test points. Errors are reported as
//   absolute error = |f(x) - P(x)|
//   relative error % = |f(x) - P(x)| / |f(x)| * 100

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <string>
#include <functional>

using namespace std;

// ---------- Lagrange interpolation core ----------
// Given n nodes (xi, yi), evaluate the Lagrange polynomial at x.
double lagrange(const vector<double>& xn, const vector<double>& yn, double x) {
    int n = xn.size();
    double result = 0.0;
    for (int i = 0; i < n; ++i) {
        double Li = 1.0;
        for (int j = 0; j < n; ++j) {
            if (j == i) continue;
            Li *= (x - xn[j]) / (xn[i] - xn[j]);
        }
        result += yn[i] * Li;
    }
    return result;
}

// Build equally-spaced nodes on [a, b], n points total.
vector<double> linspace(double a, double b, int n) {
    vector<double> v(n);
    double step = (b - a) / (n - 1);
    for (int i = 0; i < n; ++i) v[i] = a + i * step;
    return v;
}

// Pretty-print a comparison table for one function.
void analyze(const string& name,
             function<double(double)> f,
             double a, double b, int n_nodes,
             const vector<double>& test_points)
{
    cout << "\n========================================================\n";
    cout << " Function: " << name << "\n";
    cout << " Interval: [" << a << ", " << b << "],  nodes = " << n_nodes
         << " (degree = " << n_nodes - 1 << ")\n";
    cout << "========================================================\n";

    // Build nodes
    vector<double> xn = linspace(a, b, n_nodes);
    vector<double> yn(n_nodes);
    for (int i = 0; i < n_nodes; ++i) yn[i] = f(xn[i]);

    cout << "\nInterpolation nodes (xi, f(xi)):\n";
    cout << fixed << setprecision(6);
    for (int i = 0; i < n_nodes; ++i)
        cout << "  x" << i << " = " << setw(10) << xn[i]
             << "    f(x" << i << ") = " << setw(12) << yn[i] << "\n";

    // Evaluate at test points
    cout << "\n  " << setw(10) << "x"
         << "  " << setw(14) << "f(x) exact"
         << "  " << setw(14) << "P(x) interp"
         << "  " << setw(14) << "abs error"
         << "  " << setw(14) << "rel error %" << "\n";
    cout << string(76, '-') << "\n";

    double max_abs = 0.0, max_rel = 0.0;
    for (double x : test_points) {
        double fx = f(x);
        double px = lagrange(xn, yn, x);
        double err = fabs(fx - px);
        double rel = (fabs(fx) > 1e-12) ? err / fabs(fx) * 100.0 : 0.0;

        if (err > max_abs) max_abs = err;
        if (rel > max_rel) max_rel = rel;

        cout << "  " << setw(10) << x
             << "  " << setw(14) << fx
             << "  " << setw(14) << px
             << "  " << setw(14) << scientific << setprecision(3) << err
             << "  " << setw(14) << fixed << setprecision(4) << rel
             << "\n" << fixed << setprecision(6);
    }
    cout << string(76, '-') << "\n";
    cout << "  Max absolute error : " << scientific << setprecision(3) << max_abs << "\n";
    cout << "  Max relative error : " << fixed << setprecision(4) << max_rel << " %\n";
}

int main() {
    cout.setf(ios::fixed);

    // ---------- Function 1: cos(x) on [0, pi] ----------
    analyze("f1(x) = cos(x)",
            [](double x){ return cos(x); },
            0.0, M_PI, 4,
            {0.3, 0.8, 1.2, 1.5708, 2.0, 2.5, 2.9});

    // ---------- Function 2: 1/(1+x^2) on [-2, 2]  (Runge) ----------
    analyze("f2(x) = 1 / (1 + x^2)   [Runge function]",
            [](double x){ return 1.0 / (1.0 + x*x); },
            -2.0, 2.0, 4,
            {-1.8, -1.2, -0.6, 0.0, 0.6, 1.2, 1.8});

    // ---------- Function 3: ln(x) * sqrt(x) on [1, 5] ----------
    analyze("f3(x) = ln(x) * sqrt(x)",
            [](double x){ return log(x) * sqrt(x); },
            1.0, 5.0, 4,
            {1.3, 1.8, 2.4, 3.0, 3.6, 4.2, 4.7});

    cout << "\nDone.\n";
    return 0;
}
