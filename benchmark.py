import json
import os
import subprocess
import time
import catppuccin
import matplotlib.pyplot as plt
import numpy as np
import random

env = os.environ.copy()
valid_answers = "answers.txt"
program = os.path.abspath("./cmake-build-release-docker/bin/wordle_solved")
sample_size = 100

programs = {
    "C++ 1 Step": lambda word: ["./cmake-build-release-docker/bin/wordle_solved", word, "-sk", "1"],
    "C++ Fast 1 Step": lambda word: ["./cmake-build-release-docker/bin/wordle_solved", word, "-sfk", "1"],
    "C++ 2 Step": lambda word: ["./cmake-build-release-docker/bin/wordle_solved", word, "-sk", "2"],
    "C++ Fast 2 Step": lambda word: ["./cmake-build-release-docker/bin/wordle_solved", word, "-sfk", "2"],
    "Python 1 Step": lambda word: ["./.venv/bin/python", "wordle.py", "-k", "1", "-a", word],
    "PyTorch": lambda word: ["./.venv/bin/python", "model.py", "-l", "model", "-a", word],
}

with open(valid_answers, "r") as f:
    all_words = [line.strip() for line in f if line.strip()]
    words = random.sample(all_words, sample_size)

results = {label: {} for label in programs}

### Run Program
for label, func in programs.items():
    print(f"\nBenchmarking: {label}\n")
    for word in words:
        cmd = func(word)
        start_time = time.perf_counter()

        try:
            output = subprocess.check_output(cmd).decode().strip()
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

# Extract labels and average values dynamically
labels = list(results.keys())
valid_words = {w: d for w, d in word.items() if w != "Average"}
if valid_words:
    avg_guesses = sum(d["guesses"] for d in valid_words.values()) / len(valid_words)
    avg_time = sum(d["time"] for d in valid_words.values()) / len(valid_words)
    results[label]["Average"] = {"guesses": avg_guesses, "time": avg_time}
else:
    results[label]["Average"] = {"guesses": 0, "time": 0}

x = np.arange(len(labels))
width = 0.6

# Average Guesses
fig, ax = plt.subplots(constrained_layout=True)
bars = ax.bar(x, avg_guesses, width, color='skyblue')
ax.set_ylabel("Average Number of Guesses")
ax.set_title("Average Number of Guesses per Program")
ax.set_xticks(x)
ax.set_xticklabels(labels, rotation=45, ha='right')
ax.set_ylim(0, max(avg_guesses) * 1.10)

# Add value labels on top of bars
for bar in bars:
    yval = bar.get_height()
    ax.text(bar.get_x() + bar.get_width()/2, yval * 1.005, f"{yval:.2f}",
            ha='center', va='bottom')

plt.savefig("benchmark/average-guesses.png", dpi=300)

# Average Time
fig, ax = plt.subplots(constrained_layout=True)
bars = ax.bar(x, avg_time, width, color='orange')
ax.set_ylabel("Average Calculation Time (s)")
ax.set_title("Average Calculation Time per Program")
ax.set_xticks(x)
ax.set_xticklabels(labels, rotation=45, ha='right')
ax.set_ylim(0, max(avg_time) * 1.10)

for bar in bars:
    yval = bar.get_height()
    ax.text(bar.get_x() + bar.get_width()/2, yval * 1.005, f"{yval:.2f}",
            ha='center', va='bottom')

plt.savefig("benchmark/average-time.png", dpi=300)