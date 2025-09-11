import math
import time
from collections import defaultdict
from multiprocessing import Pool, cpu_count


def get_solutions(solutions=None, guess=None, feedback=None):
    if solutions:
        return [s for s in solutions if get_feedback(guess, s) == feedback]
    else:
        with open("words.txt") as f:
            return f.read().splitlines()


def get_feedback(guess, solution):
    pattern = ["B"] * 5
    chars = list(solution)

    # Greens
    for i, ch in enumerate(guess):
        if ch == solution[i]:
            pattern[i] = "G"
            chars[i] = None

    # Yellows
    for i, ch in enumerate(guess):
        if pattern[i] == "B" and ch in chars:
            pattern[i] = "Y"
            chars[chars.index(ch)] = None

    return "".join(pattern)


def get_entropy(guess, solutions):
    pattern_count = defaultdict(int)

    for solution in solutions:
        pattern = get_feedback(guess, solution)
        pattern_count[pattern] += 1

    total = len(solutions)
    entropy = 0.0

    for count in pattern_count.values():
        p = count / total
        entropy -= p * math.log2(p)

    return guess, entropy


def get_guess(solutions, all_solutions):
    tasks = [(guess, solutions) for guess in all_solutions]

    with Pool(cpu_count()) as pool:
        results = pool.starmap(get_entropy, tasks)

    return max(results, key=lambda x: x[1])


if __name__ == "__main__":
    solved = False
    print("Wordle Guesser")
    all_solutions = get_solutions()
    solutions = all_solutions
    guesses = 0

    while len(solutions) > 1:
        best_guess, best_entropy = get_guess(solutions, all_solutions)
        print(f"Guess: {best_guess.upper()} | Expected Entropy: {best_entropy:.2f} bits")

        feedback = input("Feedback (B/Y/G, or N for an invalid guess): ").upper().strip()

        # Remove invalid guesses
        if "N" in feedback:
            solutions = [s for s in solutions if s != best_guess]
            continue

        guesses += 1
        solutions = get_solutions(solutions, best_guess, feedback)

    print(f"Solution found in {guesses} guesses: {solutions[0].upper()}")
