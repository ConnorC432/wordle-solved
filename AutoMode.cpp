//
// Created by connor on 12/09/2025.
//

#include "AutoMode.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

AutoMode::AutoMode(Display &display, Feedback &feedback_instance)
    : display(display),
      feedback(feedback_instance),
      entropy(feedback) {}

int AutoMode::run(std::vector<std::string> guesses,
               std::vector<std::string> solutions,
               const std::string &answer) {
    std::unordered_map<std::string, uint8_t> guessFeedback;
    size_t guess_count = 0;

    while (solutions.size() > 1) {
        auto [best_guess, best_entropy] = entropy.get_best_guess(
            guesses, solutions, 2 - guess_count, display);

        guesses.erase(std::remove(guesses.begin(), guesses.end(), best_guess),
                            guesses.end());

        display.showOutput("Next Guess: " + best_guess + " | Expected Entropy: " + std::to_string(best_entropy));

        // Get feedback for actual answer
        uint8_t fb_encoded = feedback.get_feedback(best_guess, answer);

        solutions = feedback.get_new_solutions(solutions, best_guess, fb_encoded);

        guess_count ++;
    }

    if (!solutions.empty()) {
        display.showOutput("Solution found in " + std::to_string(guess_count) + " guesses: " + solutions[0]);
        return guess_count;
    } else {
        display.showOutput("No solution found.\n");
    }
    return -1;
}