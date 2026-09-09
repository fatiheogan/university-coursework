
"""
MATH 214 - Assignment 1
Numerical Root-Finding Methods
Author: [Emirhan Caliskan]

Implements:
  - Bisection Method
  - Fixed-Point Iteration
  - Newton's Method
"""

import math


# ─────────────────────────────────────────────
# 1.  BISECTION METHOD
# ─────────────────────────────────────────────
def bisection(f, a, b, eps=1e-6, max_iter=100):
    """
    Bisection Method for root finding.
    """
    if f(a) * f(b) >= 0:
        raise ValueError("f(a) and f(b) must have opposite signs.")

    iterates = []
    x_prev = a

    for i in range(max_iter):
        c = (a + b) / 2.0
        iterates.append(c)

        if abs(c - x_prev) < eps or abs(f(c)) < eps:
            return c, abs(f(c)), i + 1, iterates

        if f(a) * f(c) < 0:
            b = c
        else:
            a = c

        x_prev = c

    return c, abs(f(c)), max_iter, iterates


# ─────────────────────────────────────────────
# 2.  FIXED-POINT ITERATION
# ─────────────────────────────────────────────
def fixed_point(g, x0, eps=1e-6, max_iter=100):
    """
    Fixed-Point Iteration  x_{k+1} = g(x_k).
    """
    iterates = [x0]
    x = x0

    for i in range(max_iter):
        x_new = g(x)
        iterates.append(x_new)

        # Divergence check using math.isfinite
        if not math.isfinite(x_new) or abs(x_new) > 1e10:
            return x_new, float('inf'), i + 1, iterates, True

        # Simplified stopping criterion: only check difference between iterates
        diff = abs(x_new - x)
        if diff < eps:
            return x_new, diff, i + 1, iterates, False

        x = x_new

    diff = abs(iterates[-1] - iterates[-2])
    return x, diff, max_iter, iterates, False


# ─────────────────────────────────────────────
# 3.  NEWTON'S METHOD
# ─────────────────────────────────────────────
def newton(f, df, x0, eps=1e-6, max_iter=100):
    """
    Newton's Method  x_{k+1} = x_k - f(x_k)/f'(x_k).
    """
    iterates = [x0]
    x = x0

    for i in range(max_iter):
        fx = f(x)
        dfx = df(x)

        if abs(dfx) < 1e-14:
            return x, abs(fx), i + 1, iterates

        x_new = x - fx / dfx
        iterates.append(x_new)

        if abs(x_new - x) < eps or abs(f(x_new)) < eps:
            return x_new, abs(f(x_new)), i + 1, iterates

        x = x_new

    return x, abs(f(x)), max_iter, iterates
