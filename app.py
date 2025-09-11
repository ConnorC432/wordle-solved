import math
import time
from collections import defaultdict
from multiprocessing import Pool, cpu_count


def get_solutions():
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


def get_new_solutions(solutions=None, guess=None, feedback=None):
    new_solutions = [s for s in solutions if get_feedback(guess, s) == feedback]
    get_information_gain(solutions, new_solutions)
    return new_solutions


def get_information_gain(old_solutions, new_solutions):
    info_gain = math.log2(len(old_solutions)) - math.log2(len(new_solutions))
    print(f"Removed: {len(old_solutions) - len(new_solutions)} | "
          f"Information Gain: {info_gain} bits")
    return info_gain


def colour_string(word, feedback):
    string = ""
    for letter, fb in zip(word.upper(), feedback):
        match fb:
            case "G":
                string += f"\033[1;42;30m {letter} \033[0m"
            case "Y":
                string += f"\033[1;43;30m {letter} \033[0m"
            case "B":
                string += f"\033[1;100;97m {letter} \033[0m"
            case _:
                string += letter

    return string


if __name__ == "__main__":
    solved = False
    print("Wordle Solver")
    all_solutions = get_solutions()
    solutions = all_solutions
    guesses = []

    while len(solutions) > 1:
        best_guess, best_entropy = get_guess(solutions, all_solutions)
        print(f"Guess: \033[1m{best_guess.upper()}\033[0m | Expected Entropy: {best_entropy:.2f} bits")

        feedback = input("Feedback (\033[1;42;30m G \033[0m|\033[1;43;30m Y \033[0m|\033[1;100;97m B \033[0m, or \033[1;100;97m N \033[0m for an invalid guess): ").upper().strip()

        # Remove invalid guesses
        if "N" in feedback:
            all_solutions = [s for s in all_solutions if s != best_guess]
            continue

        guess = colour_string(best_guess, feedback)
        guesses.append(guess)
        print(f"\n {guess} \n")

        solutions = get_new_solutions(solutions, best_guess, feedback)

    print(f"Solution found in {len(guesses) + 1} guesses: {solutions[0].upper()}")
    print("\n")
    for guess in guesses:
        print(f" {guess} ")
    print(f" {colour_string(solutions[0].upper(), "GGGGG")} ")
