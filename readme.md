## Possible solutions
To start with, the full list of possible wordle solutions needs to be ascertained.
There are ~14.5k valid guesses, [listed here.](words.txt) <br>
There is also a shorter list of ~2.3k actual answers, but this list won't be used.

$$
S = \text{possible solutions}
$$

## Guess feedback
After a guess (g) is made, feedback (f) is given to show how close it is to the
solution (s).

$$
\text{feedback pattern} = f(g,s)
$$

Before a guess is submitted, it can be compared to all the other possible solutions
in the list.

$$
s \in S
$$

The feedback pattern will be represented as follows:
- **B** = Gray - Letter is not in the correct solution
- **Y** = Yellow - Letter is in the wrong position
- **G** = Green - Letter is in the correct position

For example, if the correct answer is "CRANE", then guessing "REACH" will provide
a feedback pattern of "YYGYB"

## Probability distribution
For each solution $s \in S$, we calculate how many times each feedback
pattern occurs. We can use then this to determine the probability that a feedback
pattern occurs.

$$
p = f(g,s)
$$
$$
P(p) = \frac{\text{number of solutions that give pattern p}}{|S|}
$$

## Entropy of distribution
Using information theory we can calculate the bits of information a guess provides.

$$
H(g) = - \sum_p P(p) \log_2 P(p)
$$

This should tell us the expected number of bits a guess will provide. <br>
Guesses with higher expected bits provide more information and are better guesses.

## Information gain
Before a guess is made, there is uncertainty of $|S|$ words. When we gain the
feedback, this can be used to narrow down the possible solutions.

$$
S_{\text{new}} = \{\, s \in S \mid f(g, s) = f_{\text{actual}} \,\}
$$

This is repeated until $|S| = 1$, and we have found the correct solution. <br>
We can also determine the actual information gain a guess has provided,
through the change in entropy.

$$
\Delta H = \log_2 |S| - \log_2 |S_{\text{new}}|
$$