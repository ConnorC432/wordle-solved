import math
from collections import defaultdict
from multiprocessing import Pool, cpu_count
import argparse
# from tqdm import tqdm


parser = argparse.ArgumentParser()
parser.add_argument('-a', '--answers', type=str, default=None, help="Correct word")
parser.add_argument('-k', '--k', type=int, default=None, help="K Steps")

args = parser.parse_args()
word = args.answers
k = args.k


def get_solutions(filename):
    with open(filename) as f:
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


def get_feedback_count(guess, solutions):
    feedback_count = defaultdict(int)

    for solution in solutions:
        feedback = get_feedback(guess, solution)
        feedback_count[feedback] += 1

    return feedback_count


def get_entropy(guess, solutions):
    total = len(solutions)
    entropy = 0.0

    feedback_count = get_feedback_count(guess, solutions)

    for count in feedback_count.values():
        p = count / total
        entropy -= p * math.log2(p)

    return entropy

def get_two_step_entropy(guess, solutions, all_solutions):
    total = len(solutions)
    first_entropy = get_entropy(guess, solutions)
    feedback_count = get_feedback_count(guess, solutions)

    second_entropy = 0.0

    for feedback, count in feedback_count.items():
        second_solutions = get_new_solutions(solutions, guess, feedback)

        best_entropy = 0.0
        for solution in all_solutions:
            entropy = get_entropy(solution, second_solutions)
            if entropy > best_entropy:
                best_entropy = entropy

        p = count / total
        second_entropy += p * best_entropy

    total_entropy = first_entropy + second_entropy

    return total_entropy

def get_n_step_entropy(guess, solutions, all_solutions, k=6):
    if len(solutions) == 1 or k == 0:
        return 0.0

    total = len(solutions)
    first_entropy = get_entropy(guess, solutions)
    feedback_count = get_feedback_count(guess, solutions)

    second_entropy = 0.0

    for feedback, count in feedback_count.items():
        second_solutions = get_new_solutions(solutions, guess, feedback)

        best_entropy = 0.0
        for solution in all_solutions:
            entropy = get_n_step_entropy(solution, second_solutions, all_solutions, k - 1)
            if entropy > best_entropy:
                best_entropy = entropy

        p = count / total
        second_entropy += p * best_entropy

    total_entropy = first_entropy + second_entropy

    return total_entropy


def get_new_solutions(solutions=None, guess=None, feedback=None):
    new_solutions = [s for s in solutions if get_feedback(guess, s) == feedback]
    return new_solutions


def get_information_gain(old_solutions, new_solutions):
    info_gain = math.log2(len(old_solutions)) - math.log2(len(new_solutions))
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


def n_step_worker(args):
    guess, solutions, all_solutions, guesses_left = args
    return get_n_step_entropy(guess, solutions, all_solutions, guesses_left)

def get_n_step_guess(solutions, all_solutions, guesses_left):
    tasks = [(guess, solutions, all_solutions, guesses_left) for guess in all_solutions]
    results = []

    with Pool(cpu_count()) as pool:
        for result in pool.imap_unordered(n_step_worker, tasks):
            results.append(result)

    best_index = results.index(max(results))
    best_guess = all_solutions[best_index]
    best_entropy = results[best_index]

    return best_guess, best_entropy


## Todo: change to auto mode interaction, benchmark against c++
if __name__ == "__main__":
    solved = False
    all_solutions = get_solutions("guesses.txt")
    solutions = get_solutions("answers.txt")
    guesses = 0

    while len(solutions) > 1:
        best_guess, best_entropy = get_n_step_guess(solutions, all_solutions, k)
        # print(f"Guess: \033[1m{best_guess.upper()}\033[0m | Expected Entropy: {best_entropy:.2f} bits")

        # feedback = input("Feedback (\033[1;42;30m G \033[0m|\033[1;43;30m Y \033[0m|\033[1;100;97m B \033[0m, or \033[1;100;97m N \033[0m for an invalid guess): ").upper().strip()
        feedback = get_feedback(best_guess, word).upper()

        # Remove invalid guesses
        # if "N" in feedback:
        #     all_solutions = [s for s in all_solutions if s != best_guess]
        #     continue
        #
        # guess = colour_string(best_guess, feedback)
        # guesses.append(guess)
        # print(f"\n {guess} \n")

        new_solutions = get_new_solutions(solutions, best_guess, feedback)
        # print(f"Information gain: {get_information_gain(solutions, new_solutions)} Bits")
        solutions = new_solutions
        # guesses_left -= 1
        guesses += 1

    # print(f"Solution found in {len(guesses) + 1} guesses: {solutions[0].upper()}")
    # print("\n")
    # for guess in guesses:
    #     print(f" {guess} ")
    # print(f" {colour_string(solutions[0].upper(), "GGGGG")} ")

    print(guesses)