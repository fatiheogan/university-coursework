import math

EPSILON = 1e-6
MAX_ITER = 100

# ---------------------------------------------
#  1. BISECTION METHOD
# ---------------------------------------------
def bisection(f, a, b, eps=EPSILON, max_iter=MAX_ITER):
    """
    Bisection method on interval [a, b].
    Returns: (root, residual, iterations, iterates)
    """
    # Check that the interval brackets a root (sign change required)
    if f(a) * f(b) > 0:
        raise ValueError(f"f(a) and f(b) must have opposite signs. f({a})={f(a):.4f}, f({b})={f(b):.4f}")

    iterates = []
    x_prev = a

    for k in range(max_iter):
        # Compute the midpoint of the current interval
        x = (a + b) / 2.0
        iterates.append(x)

        # Stop if the interval is small enough or the function value is near zero
        if abs(x - x_prev) < eps or abs(f(x)) < eps:
            return x, abs(f(x)), k + 1, iterates

        # Narrow the interval to the half that contains the root
        if f(a) * f(x) < 0:
            b = x
        else:
            a = x

        x_prev = x

    return x, abs(f(x)), max_iter, iterates


# ---------------------------------------------
#  2. FIXED-POINT ITERATION
# ---------------------------------------------
def fixed_point(g, x0, eps=EPSILON, max_iter=MAX_ITER):
    """
    Fixed-point iteration: x_{k+1} = g(x_k).
    Returns: (root, residual, iterations, iterates, converged)
    """
    iterates = [x0]
    x = x0

    for k in range(max_iter):
        # Apply the fixed-point function to get the next iterate
        x_new = g(x)
        iterates.append(x_new)

        # Stop if consecutive iterates are close enough (convergence)
        if abs(x_new - x) < eps:
            return x_new, abs(x_new - x), k + 1, iterates, True

        # Stop early if the sequence is clearly diverging
        if abs(x_new) > 1e10:
            return x_new, float('inf'), k + 1, iterates, False

        x = x_new

    # Return False if max iterations reached without convergence
    return x, abs(g(x) - x), max_iter, iterates, False


# ---------------------------------------------
#  3. NEWTON'S METHOD
# ---------------------------------------------
def newton(f, df, x0, eps=EPSILON, max_iter=MAX_ITER):
    """
    Newton's method: x_{k+1} = x_k - f(x_k)/f'(x_k).
    Returns: (root, residual, iterations, iterates, converged)
    """
    iterates = [x0]
    x = x0

    for k in range(max_iter):
        fx = f(x)
        dfx = df(x)

        # Stop if derivative is essentially zero (method breaks down)
        if abs(dfx) < 1e-14:
            print(f"  [Newton] Zero derivative at iteration {k}, stopping.")
            return x, abs(fx), k, iterates, False

        # Newton update step: move along the tangent line
        x_new = x - fx / dfx
        iterates.append(x_new)

        # Stop if the step is small or the function value is near zero
        if abs(x_new - x) < eps or abs(f(x_new)) < eps:
            return x_new, abs(f(x_new)), k + 1, iterates, True

        x = x_new

    # If max iterations reached, check if residual is small enough to count as converged
    return x, abs(f(x)), max_iter, iterates, abs(f(x)) < 1e-4
