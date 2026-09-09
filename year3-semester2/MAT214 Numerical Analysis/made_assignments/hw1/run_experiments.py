import math
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import os
from solvers import bisection, fixed_point, newton

# Create output folder for plots
os.makedirs("plots", exist_ok=True)

# =====================================================
#  FUNCTION DEFINITIONS
# =====================================================

# Problem 1: f(x) = x^3 - x - 2,  root ~ 1.52138
def f1(x): return x**3 - x - 2
def df1(x): return 3*x**2 - 1
def g1a(x): return (x + 2) ** (1/3)   # converging g(x): |g'(x*)| ~ 0.21 < 1
def g1b(x): return x**3 - 2            # diverging g(x):  |g'(x*)| ~ 6.94 > 1

TRUE_ROOT_1 = 1.5213797068045676

# Problem 2: f(x) = cos(x) - x,  root ~ 0.73909
def f2(x): return math.cos(x) - x
def df2(x): return -math.sin(x) - 1
def g2a(x): return math.cos(x)              # converging g(x): |g'(x*)| = |sin(x*)| ~ 0.674 < 1
def g2b(x): return x - 2*(math.cos(x) - x) # diverging g(x):  |g'(x*)| = |2sin(x*)+1| ~ 2.35 > 1

TRUE_ROOT_2 = 0.7390851332151607

# Problem 3: f(x) = x^3,  root = 0 (triple root)
def f3(x): return x**3
def df3(x): return 3*x**2
def g3a(x): return x - x**3   # borderline: |g'(0)| = 1, stalls near the root
def g3b(x): return x / 2.0    # converging g(x): |g'(0)| = 0.5 < 1

TRUE_ROOT_3 = 0.0

# =====================================================
#  RUN ALL EXPERIMENTS
# =====================================================
R = {}  # dictionary to store all results for plotting

W = 72
print("=" * W)
print("  MATH 214 - Assignment 1 Results".center(W))
print("  FATIH EMRE OGAN".center(W))
print("  230104004090".center(W))
print("=" * W)

# -------------------------------------------------------
# PROBLEM 1
# -------------------------------------------------------
print()
print("-" * W)
print("  PROBLEM 1 : f(x) = x^3 - x - 2       True root ~ 1.52137971".center(W))
print("-" * W)
print(f"  {'Method':<35} {'Root':>14} {'Residual':>12} {'Iters':>6}  {'Status'}")
print(f"  {'-'*35} {'-'*14} {'-'*12} {'-'*6}  {'-'*10}")

r, res, itr, its = bisection(f1, 1, 2)
R['p1_bis'] = (r, res, itr, its, True)
print(f"  {'Bisection  [1, 2]':<35} {r:>14.8f} {res:>12.2e} {itr:>6}  converged")

r, res, itr, its, cv = fixed_point(g1a, 1.5)
R['p1_fp_a'] = (r, res, itr, its, cv)
print(f"  {'Fixed-Point  g=(x+2)^(1/3)  x0=1.5':<35} {r:>14.8f} {res:>12.2e} {itr:>6}  {'converged' if cv else 'DIVERGED'}")

r, res, itr, its, cv = fixed_point(g1b, 1.5)
R['p1_fp_b'] = (r, res, itr, its, cv)
print(f"  {'Fixed-Point  g=x^3-2  x0=1.5':<35} {'---':>14} {'---':>12} {itr:>6}  DIVERGED")

r, res, itr, its, cv = newton(f1, df1, 1.5)
R['p1_newt'] = (r, res, itr, its, cv)
print(f"  {'Newton  x0=1.5':<35} {r:>14.8f} {res:>12.2e} {itr:>6}  {'converged' if cv else 'DIVERGED'}")

# -------------------------------------------------------
# PROBLEM 2
# -------------------------------------------------------
print()
print("-" * W)
print("  PROBLEM 2 : f(x) = cos(x) - x         True root ~ 0.73908513".center(W))
print("-" * W)
print(f"  {'Method':<35} {'Root':>14} {'Residual':>12} {'Iters':>6}  {'Status'}")
print(f"  {'-'*35} {'-'*14} {'-'*12} {'-'*6}  {'-'*10}")

r, res, itr, its = bisection(f2, 0, 1)
R['p2_bis'] = (r, res, itr, its, True)
print(f"  {'Bisection  [0, 1]':<35} {r:>14.8f} {res:>12.2e} {itr:>6}  converged")

r, res, itr, its, cv = fixed_point(g2a, 0.5)
R['p2_fp_a'] = (r, res, itr, its, cv)
print(f"  {'Fixed-Point  g=cos(x)  x0=0.5':<35} {r:>14.8f} {res:>12.2e} {itr:>6}  {'converged' if cv else 'DIVERGED'}")

r, res, itr, its, cv = fixed_point(g2b, 0.5)
R['p2_fp_b'] = (r, res, itr, its, cv)
print(f"  {'Fixed-Point  g=x-2(cosx-x)  x0=0.5':<35} {'---':>14} {'---':>12} {itr:>6}  DIVERGED")

# Test Newton with three different starting points to show sensitivity to initial guess
for x0, key in [(0.5, 'p2_n05'), (2.0, 'p2_n20'), (10.0, 'p2_n10')]:
    r, res, itr, its, cv = newton(f2, df2, x0)
    R[key] = (r, res, itr, its, cv)
    label = f"Newton  x0={x0}"
    print(f"  {label:<35} {r:>14.8f} {res:>12.2e} {itr:>6}  {'converged' if cv else 'DIVERGED'}")

# -------------------------------------------------------
# PROBLEM 3
# -------------------------------------------------------
print()
print("-" * W)
print("  PROBLEM 3 : f(x) = x^3  (Triple Root)  True root = 0".center(W))
print("-" * W)
print(f"  {'Method':<35} {'Root':>14} {'Residual':>12} {'Iters':>6}  {'Status'}")
print(f"  {'-'*35} {'-'*14} {'-'*12} {'-'*6}  {'-'*10}")

r, res, itr, its = bisection(f3, -1, 0.5)
R['p3_bis'] = (r, res, itr, its, True)
print(f"  {'Bisection  [-1, 0.5]':<35} {r:>14.8e} {res:>12.2e} {itr:>6}  converged")

r, res, itr, its, cv = fixed_point(g3a, 0.5)
R['p3_fp_a'] = (r, res, itr, its, cv)
print(f"  {'Fixed-Point  g=x-x^3  x0=0.5':<35} {r:>14.8e} {res:>12.2e} {itr:>6}  {'converged' if cv else 'DIVERGED'}")

r, res, itr, its, cv = fixed_point(g3b, 0.5)
R['p3_fp_b'] = (r, res, itr, its, cv)
print(f"  {'Fixed-Point  g=x/2  x0=0.5':<35} {r:>14.8e} {res:>12.2e} {itr:>6}  {'converged' if cv else 'DIVERGED'}")

# Test Newton with two starting points to show slow convergence at a triple root
for x0, key in [(0.5, 'p3_n05'), (1.0, 'p3_n10')]:
    r, res, itr, its, cv = newton(f3, df3, x0)
    R[key] = (r, res, itr, its, cv)
    label = f"Newton  x0={x0}"
    print(f"  {label:<35} {r:>14.8e} {res:>12.2e} {itr:>6}  {'converged' if cv else 'DIVERGED'}")

print()
print("=" * W)

# =====================================================
#  HELPER
# =====================================================
def errs(iterates, true_root):
    # Compute absolute error at each iteration, clamp to avoid log(0)
    return [max(abs(x - true_root), 1e-16) for x in iterates]

plt.style.use('seaborn-v0_8-whitegrid')

# =====================================================
#  PLOT 1 - Problem 1
# =====================================================
fig, axes = plt.subplots(1, 3, figsize=(15, 5))
fig.suptitle("Problem 1: f(x) = x^3 - x - 2", fontsize=14, fontweight='bold')

# Bisection: error should decrease linearly (straight line on log scale)
ax = axes[0]
e = errs(R['p1_bis'][3], TRUE_ROOT_1)
ax.semilogy(range(len(e)), e, 'b-o', markersize=4)
ax.set_title("Bisection [1, 2]")
ax.set_xlabel("Iteration")
ax.set_ylabel("|xk - x*|")

# Fixed-Point: compare converging vs diverging g(x)
ax = axes[1]
ea = errs(R['p1_fp_a'][3], TRUE_ROOT_1)
ax.semilogy(range(len(ea)), ea, 'g-o', markersize=4, label='g=(x+2)^(1/3) [conv.]')
eb = [min(max(abs(x - TRUE_ROOT_1), 1e-16), 1e13) for x in R['p1_fp_b'][3]]
ax.semilogy(range(len(eb)), eb, 'r-s', markersize=4, label='g=x^3-2 [div.]')
ax.set_title("Fixed-Point: Two g(x) Forms")
ax.set_xlabel("Iteration")
ax.set_ylabel("|xk - x*|")
ax.legend(fontsize=8)

# Newton: quadratic convergence visible as steep drop
ax = axes[2]
e = errs(R['p1_newt'][3], TRUE_ROOT_1)
ax.semilogy(range(len(e)), e, 'm-o', markersize=5)
ax.set_title("Newton x0=1.5")
ax.set_xlabel("Iteration")
ax.set_ylabel("|xk - x*|")

plt.tight_layout()
plt.savefig("plots/plot1_polynomial.png", dpi=150, bbox_inches='tight')
plt.close()
print("\n[Plot saved] plot1_polynomial.png")

# =====================================================
#  PLOT 2 - Problem 2
# =====================================================
fig, axes = plt.subplots(1, 3, figsize=(15, 5))
fig.suptitle("Problem 2: f(x) = cos(x) - x", fontsize=14, fontweight='bold')

# Bisection on [0, 1] where f(0)=1 > 0 and f(1)=cos(1)-1 < 0
ax = axes[0]
e = errs(R['p2_bis'][3], TRUE_ROOT_2)
ax.semilogy(range(len(e)), e, 'b-o', markersize=4)
ax.set_title("Bisection [0, 1]")
ax.set_xlabel("Iteration")
ax.set_ylabel("|xk - x*|")

# Fixed-Point: g=cos(x) converges, g=x-2(cos-x) diverges
ax = axes[1]
ea = errs(R['p2_fp_a'][3], TRUE_ROOT_2)
ax.semilogy(range(len(ea)), ea, 'g-o', markersize=4, label='g=cos(x) [conv.]')
eb = [min(max(abs(x - TRUE_ROOT_2), 1e-16), 1e13) for x in R['p2_fp_b'][3]]
ax.semilogy(range(len(eb)), eb, 'r-s', markersize=4, label='g=x-2(cosx-x) [div.]')
ax.set_title("Fixed-Point: Two g(x) Forms")
ax.set_xlabel("Iteration")
ax.set_ylabel("|xk - x*|")
ax.legend(fontsize=8)

# Newton: show how initial guess affects convergence speed
ax = axes[2]
colors3 = ['blue', 'green', 'red']
labels3 = ['x0=0.5', 'x0=2.0', 'x0=10.0']
keys3 = ['p2_n05', 'p2_n20', 'p2_n10']
for col, lbl, key in zip(colors3, labels3, keys3):
    entry = R[key]
    if entry[4]:
        # Converged: plot error from true root
        e = errs(entry[3], TRUE_ROOT_2)
        ax.semilogy(range(len(e)), e, '-o', markersize=4, color=col, label=lbl + " [conv.]")
    else:
        # Diverged: plot |f(xk)| as a residual indicator
        res_vals = [max(abs(f2(x)), 1e-16) for x in entry[3]]
        ax.semilogy(range(len(res_vals)), res_vals, '--s', markersize=3,
                    color=col, alpha=0.8, label=lbl + " [div., |f(xk)|]")
ax.set_title("Newton: Different x0")
ax.set_xlabel("Iteration")
ax.set_ylabel("|xk - x*| or |f(xk)|")
ax.legend(fontsize=7)

plt.tight_layout()
plt.savefig("plots/plot2_transcendental.png", dpi=150, bbox_inches='tight')
plt.close()
print("[Plot saved] plot2_transcendental.png")

# =====================================================
#  PLOT 3 - Problem 3
# =====================================================
fig, axes = plt.subplots(1, 3, figsize=(15, 5))
fig.suptitle("Problem 3: f(x) = x^3  (Triple Root at 0)", fontsize=14, fontweight='bold')

# Bisection on [-1, 0.5]: f(-1) < 0, f(0.5) > 0
ax = axes[0]
e = errs(R['p3_bis'][3], TRUE_ROOT_3)
ax.semilogy(range(len(e)), e, 'b-o', markersize=4)
ax.set_title("Bisection [-1, 0.5]")
ax.set_xlabel("Iteration")
ax.set_ylabel("|xk - x*|")

# Fixed-Point: g=x/2 converges, g=x-x^3 stalls at |g'(0)|=1
ax = axes[1]
ea = errs(R['p3_fp_a'][3], TRUE_ROOT_3)
ax.semilogy(range(len(ea)), ea, 'g-o', markersize=3, label='g=x-x^3 [stalls]')
eb = errs(R['p3_fp_b'][3], TRUE_ROOT_3)
ax.semilogy(range(len(eb)), eb, 'r-s', markersize=4, label='g=x/2 [conv.]')
ax.set_title("Fixed-Point: Two g(x) Forms")
ax.set_xlabel("Iteration")
ax.set_ylabel("|xk - x*|")
ax.legend(fontsize=8)

# Newton: shows linear convergence at triple root instead of quadratic
# Theoretical rate for multiplicity-3 root is r = 1 - 1/3 = 2/3
ax = axes[2]
e05 = errs(R['p3_n05'][3], TRUE_ROOT_3)
e10 = errs(R['p3_n10'][3], TRUE_ROOT_3)
ax.semilogy(range(len(e05)), e05, 'b-o', markersize=4, label='x0=0.5')
ax.semilogy(range(len(e10)), e10, 'g-o', markersize=4, label='x0=1.0')
theory = [max(e05[0] * (2/3)**k, 1e-16) for k in range(len(e05))]
ax.semilogy(range(len(theory)), theory, 'k--', alpha=0.6, label='Theory r=2/3')
ax.set_title("Newton: Slow Conv. (Triple Root)")
ax.set_xlabel("Iteration")
ax.set_ylabel("|xk - x*|")
ax.legend(fontsize=8)

plt.tight_layout()
plt.savefig("plots/plot3_triple_root.png", dpi=150, bbox_inches='tight')
plt.close()
print("[Plot saved] plot3_triple_root.png")

print("\nAll done.")
