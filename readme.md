## Possible solutions
To start with, the full list of possible wordle solutions needs to be ascertained.
There are ~14.5k valid guesses, [listed here.](words.txt) <br>
There is also a shorter list of ~2.3k actual answers, [found here.](validwords.txt) <br>
While it may seem more efficient to only use the shorter list, both lists will be utilised.
The shorter list will be used to narrow down the valid answers initially, while the larger list
can still be used to gather information.

$$
S = \text{possible solutions}
$$

```
def get_solutions():
    with open("words.txt") as f:
        return f.read().splitlines()
```

## Guess feedback
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

## Probability distribution
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
pattern_count = defaultdict(int)

for solution in solutions:
    pattern = get_feedback(guess, solution)
    pattern_count[pattern] += 1
    
total = len(solutions)

for count in pattern_count.values():
    p = count / total
```

## Entropy of distribution
Using information theory we can calculate the bits of information a guess provides.

$$
H(g) = - \sum_p P(p) \log_2 P(p)
$$

```
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
```

This should tell us the expected number of bits a guess will provide. <br>
Guesses with higher expected bits provide more information and are better guesses.

## Information gain
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

## Second Guess Entropy
To establish a more useful guess