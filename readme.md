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