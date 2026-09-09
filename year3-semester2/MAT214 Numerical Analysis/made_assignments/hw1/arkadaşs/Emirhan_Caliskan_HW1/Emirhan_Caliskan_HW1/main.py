# MATH 214 - Assignment 1
# Main script: runs all experiments, prints results table, generates plots.

import math
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import os

from solvers import bisection, fixed_point, newton

# ── Output directory ──────────────────────────────────────────────────────────
os.makedirs("figures", exist_ok=True)

EPS   = 1e-6

# Hata Düzeltildi: Problem 1 için doğru gerçek kök girildi.
TRUE_ROOTS = {
    "poly":   1.5213797068,   # x^3 - x - 2 = 0
    "trans":  0.7390851332,   # cos(x) - x = 0
    "cubic":  0.0,            # x^3 = 0
}

results = []  # collect rows for the table


# ─────────────────────────────────────────────────────────────────────────────
# PROBLEM 1 – f(x) = x^3 - x - 2
# ─────────────────────────────────────────────────────────────────────────────
print("=" * 70)
print("PROBLEM 1: f(x) = x^3 - x - 2   (true root ≈ 1.5213797068)")
print("=" * 70)

f1  = lambda x: x**3 - x - 2
df1 = lambda x: 3*x**2 - 1
x_star = TRUE_ROOTS["poly"]

# ── 1a. Bisection [1, 2] ─────────────────────────────────────────────────────
root, res, n, iters = bisection(f1, 1, 2, EPS)
errors = [abs(xi - x_star) for xi in iters]
print(f"\n[Bisection]  root={root:.8f}  residual={res:.2e}  iters={n}")
results.append(("x^3-x-2", "Bisection", "[1,2]", n, root, res))

plt.figure(figsize=(7, 4))
plt.semilogy(range(1, len(errors)+1), errors, 'b-o', markersize=4)
plt.xlabel("Iteration"); plt.ylabel("|x_k - x*|")
plt.title("Problem 1 – Bisection: Error vs Iteration")
plt.grid(True, which='both', ls='--', alpha=0.5)
plt.tight_layout()
plt.savefig("figures/p1_bisection.png", dpi=150)
plt.close()

# ── 1b. Fixed-Point – g1(x) = (x + 2)^(1/3)  (converges) ───────────────────
g1a = lambda x: (x + 2) ** (1/3)
root_fp1, res_fp1, n_fp1, iters_fp1, div1 = fixed_point(g1a, 1.5, EPS)
errors_fp1 = [abs(xi - x_star) for xi in iters_fp1]
status1 = "DIVERGED" if div1 else "OK"
print(f"[FP g1=(x+2)^(1/3)]  root={root_fp1:.8f}  residual={res_fp1:.2e}  iters={n_fp1}  [{status1}]")
results.append(("x^3-x-2", "Fixed-Point g=(x+2)^(1/3)", "x0=1.5", n_fp1, root_fp1, res_fp1))

# ── 1c. Fixed-Point – g2(x) = x^3 - 2  (diverges) ──────────────────────────
g1b = lambda x: x**3 - 2
root_fp2, res_fp2, n_fp2, iters_fp2, div2 = fixed_point(g1b, 1.5, EPS)
errors_fp2_raw = [abs(xi - x_star) for xi in iters_fp2]

# math.isfinite kullanıldı
errors_fp2 = [e for e in errors_fp2_raw if math.isfinite(e) and e < 1e8]
status2 = "DIVERGED" if div2 else "OK"
print(f"[FP g2=x^3-2      ]  root={root_fp2:.3g}  residual={res_fp2:.2e}  iters={n_fp2}  [{status2}]")
results.append(("x^3-x-2", "Fixed-Point g=x^3-2 (diverges)", "x0=1.5", n_fp2, "N/A", "∞"))

plt.figure(figsize=(7, 4))
plt.semilogy(range(1, len(errors_fp1)+1), errors_fp1, 'g-s', markersize=4, label=r"$g_1=(x+2)^{1/3}$ (converges)")
if errors_fp2:
    plt.semilogy(range(1, len(errors_fp2)+1), errors_fp2, 'r-^', markersize=4, label=r"$g_2=x^3-2$ (diverges)")
plt.xlabel("Iteration"); plt.ylabel("|x_k - x*|")
plt.title("Problem 1 – Fixed-Point: Two g(x) Forms")
plt.legend(); plt.grid(True, which='both', ls='--', alpha=0.5)
plt.tight_layout()
plt.savefig("figures/p1_fixedpoint.png", dpi=150)
plt.close()

# ── 1d. Newton's Method ──────────────────────────────────────────────────────
root_n1, res_n1, n_n1, iters_n1 = newton(f1, df1, 1.5, EPS)
errors_n1 = [abs(xi - x_star) for xi in iters_n1]
print(f"[Newton]             root={root_n1:.8f}  residual={res_n1:.2e}  iters={n_n1}")
results.append(("x^3-x-2", "Newton", "x0=1.5", n_n1, root_n1, res_n1))

# Combined convergence plot for Problem 1
plt.figure(figsize=(7, 4))
plt.semilogy(range(1, len(errors)+1), errors, 'b-o', markersize=4, label="Bisection")
plt.semilogy(range(1, len(errors_fp1)+1), errors_fp1, 'g-s', markersize=4, label=r"FP $g_1$")
plt.semilogy(range(1, len(errors_n1)+1), errors_n1, 'r-^', markersize=4, label="Newton")
plt.xlabel("Iteration"); plt.ylabel("|x_k - x*|")
plt.title("Problem 1 – All Methods Comparison")
plt.legend(); plt.grid(True, which='both', ls='--', alpha=0.5)
plt.tight_layout()
plt.savefig("figures/p1_all_methods.png", dpi=150)
plt.close()


# ─────────────────────────────────────────────────────────────────────────────
# PROBLEM 2 – f(x) = cos(x) - x
# ─────────────────────────────────────────────────────────────────────────────
print("\n" + "=" * 70)
print("PROBLEM 2: f(x) = cos(x) - x   (true root ≈ 0.7390851332)")
print("=" * 70)

# math kütüphanesi kullanıldı
f2  = lambda x: math.cos(x) - x
df2 = lambda x: -math.sin(x) - 1
x_star2 = TRUE_ROOTS["trans"]

initial_guesses = [0.0, 0.5, 1.0, 3.0]
colors = ['b', 'g', 'r', 'm']
plt.figure(figsize=(7, 4))

for x0, col in zip(initial_guesses, colors):
    root_n2, res_n2, n_n2, iters_n2 = newton(f2, df2, x0, EPS)
    errors_n2 = [abs(xi - x_star2) for xi in iters_n2]
    errors_n2 = [e for e in errors_n2 if e > 0 and math.isfinite(e)]
    print(f"[Newton x0={x0:4.1f}]  root={root_n2:.8f}  residual={res_n2:.2e}  iters={n_n2}")
    results.append(("cos(x)-x", f"Newton x0={x0}", f"x0={x0}", n_n2, root_n2, res_n2))
    if errors_n2:
        plt.semilogy(range(1, len(errors_n2)+1), errors_n2, f'{col}-o', markersize=4, label=f"x0={x0}")

plt.xlabel("Iteration"); plt.ylabel("|x_k - x*|")
plt.title("Problem 2 – Newton's Method: Effect of Initial Guess")
plt.legend(); plt.grid(True, which='both', ls='--', alpha=0.5)
plt.tight_layout()
plt.savefig("figures/p2_newton_cos.png", dpi=150)
plt.close()

# Bisection and Fixed-Point for completeness
root_b2, res_b2, n_b2, iters_b2 = bisection(f2, 0, 1, EPS)
print(f"[Bisection  [0,1]]   root={root_b2:.8f}  residual={res_b2:.2e}  iters={n_b2}")
results.append(("cos(x)-x", "Bisection", "[0,1]", n_b2, root_b2, res_b2))

# math.cos kullanıldı
g2 = lambda x: math.cos(x)
root_fp3, res_fp3, n_fp3, iters_fp3, div3 = fixed_point(g2, 0.5, EPS)
print(f"[FP g=cos(x)]        root={root_fp3:.8f}  residual={res_fp3:.2e}  iters={n_fp3}")
results.append(("cos(x)-x", "Fixed-Point g=cos(x)", "x0=0.5", n_fp3, root_fp3, res_fp3))


# ─────────────────────────────────────────────────────────────────────────────
# PROBLEM 3 – f(x) = x^3   (multiple root at 0)
# ─────────────────────────────────────────────────────────────────────────────
print("\n" + "=" * 70)
print("PROBLEM 3: f(x) = x^3   (triple root at x = 0)")
print("=" * 70)

f3  = lambda x: x**3
df3 = lambda x: 3*x**2
x_star3 = TRUE_ROOTS["cubic"]

x0_vals = [1.0, 0.5, 0.1]
plt.figure(figsize=(7, 4))

for x0, col in zip(x0_vals, ['b', 'g', 'r']):
    root_n3, res_n3, n_n3, iters_n3 = newton(f3, df3, x0, EPS)
    errors_n3 = [abs(xi - x_star3) for xi in iters_n3]
    errors_n3 = [e for e in errors_n3 if e > 0 and math.isfinite(e)]
    print(f"[Newton x0={x0}]  root={root_n3:.2e}  residual={res_n3:.2e}  iters={n_n3}")
    results.append(("x^3 (mult. root)", f"Newton x0={x0}", f"x0={x0}", n_n3, root_n3, res_n3))

    if errors_n3:
        plt.semilogy(range(1, len(errors_n3)+1), errors_n3, f'{col}-o', markersize=4, label=f"x0={x0}")

plt.xlabel("Iteration"); plt.ylabel("|x_k - 0|")
plt.title("Problem 3 – Newton on x^3: Linear Convergence at Triple Root")
plt.legend(); plt.grid(True, which='both', ls='--', alpha=0.5)
plt.tight_layout()
plt.savefig("figures/p3_newton_cubic.png", dpi=150)
plt.close()

# Convergence rate analysis: ratio test
print("\n[Convergence rate analysis for x^3, x0=1.0]")
_, _, _, iters_rate = newton(f3, df3, 1.0, EPS)
errs = [abs(xi - 0) for xi in iters_rate if abs(xi) > 0]
print("  k   |x_k|          ratio |x_{k+1}|/|x_k|")
for k in range(len(errs)-1):
    if errs[k] > 0:
        ratio = errs[k+1] / errs[k]
        print(f"  {k+1:2d}  {errs[k]:.6e}     {ratio:.4f}")

# Bisection for Problem 3
root_b3, res_b3, n_b3, iters_b3 = bisection(f3, -1, 1, EPS)
print(f"[Bisection [-1,1]]  root={root_b3:.2e}  residual={res_b3:.2e}  iters={n_b3}")
results.append(("x^3 (mult. root)", "Bisection", "[-1,1]", n_b3, root_b3, res_b3))


# ─────────────────────────────────────────────────────────────────────────────
# RESULTS TABLE
# ─────────────────────────────────────────────────────────────────────────────
print("\n" + "=" * 90)
print(f"{'Function':<20} {'Method':<35} {'Init':<10} {'Iters':>5}  {'Root':>14}  {'Residual':>12}")
print("=" * 90)
for r in results:
    fn, meth, init, iters, root, res = r
    if isinstance(root, float):
        root_str = f"{root:.8f}"
    else:
        root_str = str(root)
    if isinstance(res, float):
        res_str = f"{res:.2e}"
    else:
        res_str = str(res)
    print(f"{fn:<20} {meth:<35} {init:<10} {iters:>5}  {root_str:>14}  {res_str:>12}")
print("=" * 90)
print("\nAll figures saved to ./figures/")