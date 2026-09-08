import random


def determine_the_winner(n, m):
    # Simulate One-Pile NIM game and calculates the winner.
    # The first player will win the game with given constraint

    if n % (m + 1) == 0:  # Check constraint.
        print("n mod(m+1) cannot equal to 0.")
    elif m > n:
        print("m cannot be greater than n.")
    else:
        print("The chips count: {}, at most: {}".format(n, m))
        print("-" * 54)
        player = 0

        # Iterates till the end of game.
        while n != 0:
            # Change turn.
            player = (player + 1) % 2

            # This is the key place for algorithm.
            # First player must take n % (m + 1) chips to win the game.
            # The last turn will remain n <= m chips and it is taken from
            # first player.
            if player == 0:
                if n != m:
                    move = n % (m + 1)
                else:
                    move = n
            # Generate random move for second player
            else:
                move = random.randint(1, m)
            n -= move

            # Print in each iteration on the screen.
            print("{} took {} chips. Remaining chips: {} - {} = {}".format(
                "First Player" if player == 0 else "Second Player", move,
                move + n, move, n))

        print("=============== {} has won! ===============".format(
            "First Player" if player == 0 else "Second Player"))
        print("*" * 54)


if __name__ == "__main__":
    n, m = random.randint(56, 300), random.randint(1, 28)
    determine_the_winner(n, m)

# Algorithm Analysis:
# At each turn, input size decreases 1 to m;
# on the worst case the algortihm takes 1 chip at each turn.
# Thus, running time at worst case is O(n)


