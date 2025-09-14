import json
import os
import subprocess
import time
import catppuccin
import matplotlib.pyplot as plt
import numpy as np

env = os.environ.copy()
valid_answers = "answers.txt"
program = os.path.abspath("./cmake-build-release-docker/bin/wordle_solved")

args = {
    "1 Step": ["-sk", "1"],
    "Fast 1 Step": ["-sfk", "1"],
    "2 Step": ["-sk", "2"],
    "Fast 2 Step": ["-sfk", "2"]
}

with open(valid_answers, "r") as f:
    words = [line.strip() for line in f if line.strip()]

results = {label: {} for label in args}

### Run Program
for label, arg in args.items():
    print(f"\nBenchmarking with args: {arg}\n")

    for word in words:
        start_time = time.perf_counter()

        try:
            output = subprocess.check_output(
                [program, word] + arg
            ).strip()
        except subprocess.CalledProcessError as e:
            print(f"Subprocess error: {e}")
            continue

        elapsed = time.perf_counter() - start_time

        if output.isdigit():
            guesses = int(output)
            if guesses > 0:
                results[label][word] = {"guesses": guesses, "time": elapsed}
                print(f"Guessed {word}: {guesses} guesses in {elapsed:.2f} seconds.")

        else:
            print(f"Invalid output: {output}")
            continue

### Get Averages
for label, word in results.items():
    if word:
        avg_guesses = sum(d["guesses"] for d in word.values()) / len(word)
        avg_time = sum(d["time"] for d in word.values()) / len(word)

        results[label]["Average"] = {"guesses": avg_guesses, "time": avg_time}

    else:
        print(f"No valid results for {label}")
## Save Data to JSON
with open("benchmark/results.json", "w") as f:
    json.dump(results, f, indent=4, sort_keys=True)


### Process Data
plt.style.use(catppuccin.PALETTE.macchiato.identifier)
plt.tight_layout()
groups = ["1 Step", "2 Step"]
slow_labels = ["1 Step", "2 Step"]
fast_labels = ["Fast 1 Step", "Fast 2 Step"]

slow_avg_guesses = [results[label]["Average"]["guesses"] for label in slow_labels]
fast_avg_guesses = [results[label]["Average"]["guesses"] for label in fast_labels]

slow_avg_times = [results[label]["Average"]["time"] for label in slow_labels]
fast_avg_times = [results[label]["Average"]["time"] for label in fast_labels]

x = np.arange(len(groups))
width = 0.25

## Average Guesses
# Plot bars
fig, ax = plt.subplots(constrained_layout=True)
bars_slow = ax.bar(x - width/2, slow_avg_guesses, width, label="~14.8k Guess Pool")
bars_fast = ax.bar(x + width/2, fast_avg_guesses, width, label="~2.3k Guess Pool")
ax.set_ylim(0, max(slow_avg_guesses + fast_avg_guesses) * 1.10)

# Add value labels on top of bars
for bar in bars_slow + bars_fast:
    yval = bar.get_height()
    ax.text(bar.get_x() + bar.get_width()/2, yval * 1.005, f"{yval:.2f}", ha='center', va='bottom')

# Labels and title
ax.set_xlabel("Number of Steps ($k$)")
ax.set_ylabel("Average Number of Guesses")
ax.set_title("Average Number of Guesses per Number of Steps")
ax.set_xticks(x)
ax.set_xticklabels(groups)
ax.legend()

plt.savefig("benchmark/average-guesses.png", dpi=300)

## Average Time
# Plot bars
fig, ax = plt.subplots(constrained_layout=True)
bars_slow = ax.bar(x - width/2, slow_avg_times, width, label="~14.8k Guess Pool")
bars_fast = ax.bar(x + width/2, fast_avg_times, width, label="~2.3k Guess Pool")
ax.set_ylim(0, max(slow_avg_times + fast_avg_times) * 1.10)

# Add value labels on top of bars
for bar in bars_slow + bars_fast:
    yval = bar.get_height()
    ax.text(bar.get_x() + bar.get_width()/2, yval * 1.005, f"{yval:.2f}", ha='center', va='bottom')

# Labels and title
ax.set_xlabel("Number of Steps ($k$)")
ax.set_ylabel("Average Calculation Time (s)")
ax.set_title("Average Calculation Time per Number of Steps")
ax.set_xticks(x)
ax.set_xticklabels(groups)
ax.legend()

plt.savefig("benchmark/average-time.png", dpi=300)