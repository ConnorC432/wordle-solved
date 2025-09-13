import os
import subprocess
import time
import matplotlib.pyplot as plt
from collections import Counter, defaultdict

env = os.environ.copy()
valid_answers = "answers.txt"
program = os.path.abspath("./cmake-build-release/bin/wordle_solved")
results = {}
times = defaultdict(list)

with open(valid_answers, "r") as f:
    words = [line.strip() for line in f if line.strip()]

for word in words:
    try:
        start_time = time.time()
        output = subprocess.check_output(
            [program, word, "--silent"],
            text=True,
            env=env
        ).strip()
        elapsed_time = time.time() - start_time

        if output.isdigit():
            guesses = int(output)
            if 1 <= guesses <= 6:
                results[word] = guesses
                times[guesses].append(elapsed_time)
                print(f"Solved answer \"{word}\" in {guesses} guesses, taking {elapsed_time:.2f} seconds.")
            else:
                print(f"Invalid output for answer {word}")
        else:
            print(f"Invalid output for answer {word}")

    except subprocess.CalledProcessError as e:
        print(f"Subprocess error: {e}")

    except ValueError as e:
        print(f"ValueError: {e}")

if results:
    total = sum(results.values())
    average_guesses = total / len(results)
    print(f"\n\n\nAverage number of guesses is: {average_guesses}")

    average_times = {g: sum(t_list) / len(t_list) for g, t_list in times.items() if t_list}

    guess_counts = Counter(results.values())

    # Guess Distribution
    plt.bar(guess_counts.keys(), guess_counts.values(), color="skyblue")
    plt.xlabel("Number of Guesses")
    plt.ylabel("Number of Words")
    plt.title("Distribution of guesses taken")
    plt.xticks(range(1,7))
    plt.savefig("guess_distribution.png", dpi=300, bbox_inches="tight")

    # Average Calculation Time
    plt.bar(average_times.keys(), average_times.values(), color="skyblue")
    plt.xlabel("Number of Guesses")
    plt.ylabel("Time (seconds)")
    plt.title("Average Calculation Time per Number of Guesses")
    plt.xticks(range(1,7))
    plt.savefig("time_distribution.png", dpi=300, bbox_inches="tight")

    print("Benchmark done")
else:
    print("No valid data found")