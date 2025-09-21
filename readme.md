## Information Theory
### Possible solutions
To start with, the full list of possible wordle solutions needs to be ascertained.
There are ~14.5k valid guesses, [listed here.](guesses.txt) <br>
There is also a shorter list of ~2.3k actual answers, [found here.](answers.txt) <br>
While it may seem more efficient to only use the shorter list, both lists will be utilised.
The shorter list will be used to narrow down the valid answers initially, while the larger list
can still be used to gather information.

$$
S = \text{possible solutions}
$$

```
def get_solutions(filename):
    with open(filename) as f:
        return f.read().splitlines()
```

### Guess feedback
After a guess (g) is made, feedback (f) is given to show how close it is to the
solution (s).

$$
\text{feedback pattern} = f(g,s)
$$

```
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
```

The feedback pattern will be represented as follows:
- **B** = Gray - Letter is not in the correct solution
- **Y** = Yellow - Letter is in the wrong position
- **G** = Green - Letter is in the correct position

For example, if the correct answer is "CRANE", then guessing "REACH" will provide
a feedback pattern of "YYGYB"

### Probability distribution
Before a guess is submitted, it can be compared to all the other possible solutions
in the list.

$$
s \in S
$$

For each solution $s \in S$, we calculate how many times each feedback
pattern occurs. We can use then this to determine the probability that a feedback
pattern occurs.

$$
p = f(g,s)
$$
$$
P(p) = \frac{\text{number of solutions that give pattern p}}{|S|}
$$

```
def get_feedback_count(guess, solutions):
    feedback_count = defaultdict(int)
    
    for solution in solutions:
        feedback = get_feedback(guess, solution)
        feedback_count[feedback] += 1
        
    return feedback_count
```

### Entropy of distribution
Using information theory we can calculate the bits of information a guess provides.

$$
H(g) = - \sum_p P(p) \log_2 P(p)
$$

```
def get_entropy(guess, solutions):
    total = len(solutions)
    entropy = 0.0
    
    feedback_count = get_feedback_count(guess, solutions)

    for count in feedback_count.values():
        p = count / total
        entropy -= p * math.log2(p)

    return entropy
```

This should tell us the expected number of bits a guess will provide. <br>
Guesses with higher expected bits provide more information and are better guesses.

### Information gain
Before a guess is made, there is uncertainty of $|S|$ words. When we gain the
feedback, this can be used to narrow down the possible solutions.

$$
S_{\text{new}} = \{\, s \in S \mid f(g, s) = f_{\text{actual}} \,\}
$$

```
def get_new_solutions(solutions=None, guess=None, feedback=None):
    new_solutions = [s for s in solutions if get_feedback(guess, s) == feedback]
    return new_solutions
```

This is repeated until $|S| = 1$, and we have found the correct solution. <br>
We can also determine the actual information gain a guess has provided,
through the change in entropy.

$$
\Delta H = \log_2 |S| - \log_2 |S_{\text{new}}|
$$

```
def get_information_gain(old_solutions, new_solutions):
    info_gain = math.log2(len(old_solutions)) - math.log2(len(new_solutions))
    return info_gain
```

### Second Step Entropy
Currently, the next guess is chosen based on its immediate expected information gain.
A better way to determine the best guess would consider not just the immediate
information gain, but the gain over subsequent guesses as well. <br>
Deciding the best guess this way should hopefully maximise the average information
gain over multiple guesses. <br>
To achieve this we can expand on the first entropy calculation.

$$
H_1(g_1) = - \sum_f P(f \mid g_1) \log_2 P(f \mid g_1)
$$

Then for each possible feedback ($3^5 = 243$), we determine the next best possible 
entropy.

$$
H_{\text{best}}(S_f) = \max_{g_2 \in \text{all guesses}} H(g_2, S_f)
$$
where
$$
S_f = \{ s \in S \mid f(g_1, s) = f \}
$$

Then we can determine the expected second-guess entropy. This is the sum of the best
entropies for all possible feedbacks, each of them weighted by the probability of that
feedback occurring from the first guess.

$$
H_2(g_1) = \sum_f P(f \mid g_1) \cdot H_{\text{best}}(S_f)
$$
$$
H_{\text{total}} = H_1 + H_2
$$

```
def get_two_step_entropy(guess, solutions, all_solutions):
    total = len(solutions)
    first_entropy = get_entropy(guess, solutions)
    feedback_count = get_feedback_count(guess, solutions)

    second_entropy = 0.0

    for feedback, count in feedback_count.items():
        second_solutions = get_new_solutions(solutions, guess, feedback)

        best_entropy = 0.0ex
        for solution in all_solutions:
            entropy = get_entropy(solution, second_solutions)
            if entropy > best_entropy:
                best_entropy = entropy

        p = count / total
        second_entropy += p * best_entropy

    total_entropy = first_entropy + second_entropy

    return total_entropy
```

### N-Step Entropy
To further maximise the average guess information gain, multiple steps can be calculated. <br>

Let $|S|$ be the current set of possible solutions and $k$ be the number of remaining
guesses, _including_ the current guess, starting at $k = 6$.

$$
H^{(k)}(g, S) \;=\; H(g, S) \;+\; \sum_{f} P(f \mid g)\,\max_{g' \in \text{all guesses}} H^{(k-1)}(g', S_f)
$$
Where
$$
S_f \;=\; \{\, s \in S \mid f(g,s)=f \,\},
\qquad
H(g,S) \;=\; -\sum_f P(f\mid g)\log_2 P(f\mid g).
$$
Base cases:
$$
|S| = 1 \implies H^{(k)}(g,S) = 0 \qquad\text{for any }k,
$$
$$
k = 0 \implies H^{(0)}(g,S) = 0 \qquad\text{(no guesses left).}
$$

```
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
```

## Optimisation
### Getting Feedback
When the best guess is being determined, the code determines the feedback pattern
for all possible answers, for all possible guesses. For the first step, we have a list
of ~14.5k guesses and ~2.3k answers.

$$
\text{guesses} \cdot \text{answers} = 14,855 \cdot 2,315 = 34,389,325
$$

at ~34.3 million possible guess and answer combinations, the feedback determination
needs to be as fast as possible. <br>
To start with, the feedback strings, such as "GYBYG", can be replaced with a Base 3 integer.
- Gray = 0
- Yellow = 1
- Green = 2

So the previous "GYBYG" string would be:
$$
(2\cdot3^4) + (1\cdot3^3) + (0\cdot3^2) + (1\cdot3^1) + (2\cdot3^0) = 194
$$
With 243 possible combinations, this can be stored in an 8 bit integer.
```
uint8_t Feedback::get_feedback(const std::string &guess, const std::string &solution) const {
    uint8_t feedback = 0;
    int freq[26] = {0};

    // Step 1: Count letters in solution
    for (int i = 0; i < 5; ++i)
        freq[solution[i] - 'a']++;

    // Step 2: Mark greens (and adjust freq)
    uint8_t codes[5];
    for (int i = 0; i < 5; ++i) {
        if (guess[i] == solution[i]) {
            codes[i] = 2;
            freq[guess[i] - 'a']--;
        } else {
            codes[i] = 0;
        }
    }

    // Step 3: Mark yellows
    for (int i = 0; i < 5; ++i) {
        if (codes[i] == 0) {
            int idx = guess[i] - 'a';
            if (freq[idx] > 0) {
                codes[i] = 1;
                freq[idx]--;
            }
        }
    }

    // Step 4: Encode to base-3 number
    for (int i = 0; i < 5; ++i)
        feedback = feedback * 3 + codes[i];

    return feedback;
}
```

While this is a more efficient way of storing the feedback data,
the feedback determination of a guess and answer combination can
be made even faster by caching a table of all guess and answer
combinations once, and caching their respective feedback.
```
void Feedback::precache_feedback(const std::vector<std::string> &all_solutions) {
    size_t n_words = all_solutions.size();

    // Build word_index for fast lookup
    word_index.clear();
    for (size_t i = 0; i < n_words; ++i)
        word_index[all_solutions[i]] = i;

    feedback_cache.assign(n_words, std::vector<uint8_t>(n_words, 0));
    std::atomic<size_t> progress(0);

    size_t n_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;

    std::atomic<bool> done(false);
    auto start_time = std::chrono::high_resolution_clock::now();

    // Progress Bar Thread
    std::thread progress_thread([&] {
        while (!done) {
            {
                std::lock_guard<std::mutex> lock(display.getMutex());
                display.showProgress("Caching Feedback", progress.load(), n_words);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    // Worker Threads
    auto worker = [&](size_t start, size_t end) {
        for (size_t i = start; i < end; ++i) {
            for (size_t j = 0; j < n_words; ++j)
                feedback_cache[i][j] = get_feedback(all_solutions[i], all_solutions[j]);
            progress.fetch_add(1, std::memory_order_relaxed);
        }
    };

    // Split Work into Chunks
    size_t chunk_size = (n_words + n_threads - 1) / n_threads;
    for (size_t i = 0; i < n_threads; ++i) {
        size_t start = i * chunk_size;
        size_t end = std::min((start + chunk_size), n_words);
        threads.emplace_back(worker, start, end);
    }

    for (auto &t : threads) t.join();
    done = true;
    progress_thread.join();

    std::lock_guard<std::mutex> lock(display.getMutex());
    display.showProgress("Caching Feedback", n_words, n_words);
}
```

### Determining Entropy
To determine the entropy we include this as part of the calculation:
$$
\log_2 P(p)
$$
This will be a pretty computationally expensive instruction, so to
speed it up, the $log_2$ values can be cached
```
std::vector<double> Entropy::log_cache;

void Entropy::precache_log(size_t max_size,
                    Display& display) {
    log_cache.resize(max_size + 1);

    for (size_t i = 1; i <= max_size; ++i) {
        double p = static_cast<double>(i) / max_size;
        log_cache[i] = std::log2(p);

        if (i % 100 == 0 || i == max_size) {
            display.showProgress("Caching log2", i, max_size);
        }
    }
}
```

### First Best Guess
Before we make a guess, we start with no information on the answer.
Therefore the first best guess determined should always be the same. <br>
Using a 2 Step Entropy value, the word with the best expected entropy
is "Slate". The lengthy first guess determination can be skipped and
instead always produce "Slate" as the best guess.
```
std::pair<std::string, double> Entropy::get_best_guess(
    const std::vector<std::string> &guesses,
    const std::vector<std::string> &solutions,
    int k,
    Display &display
) const {
    if (std::find(guesses.begin(), guesses.end(), "slate") != guesses.end()) {
        return {"slate", 0.0};
    }

    display.showProgress("Calculating Best Guess", 0, guesses.size());
    std::atomic<size_t> progress(0);
    bool done = false;

    // Progress thread
    std::thread progress_thread([&]() {
        while (!done) {
            display.showProgress("Calculating Best Guess", progress.load(), guesses.size());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        display.showProgress("Calculating Best Guess", guesses.size(), guesses.size());
    });

    auto entropies = get_n_step_entropy(guesses, solutions, k, progress, true);

    done = true;
    progress_thread.join();

    auto max_it = std::max_element(
        entropies.begin(),
        entropies.end(),
        [](const auto &a, const auto &b) { return a.second < b.second; }
    );

    return *max_it;
}
```

## PyTorch


## Benchmarking