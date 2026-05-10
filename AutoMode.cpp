//
// Created by connor on 12/09/2025.
//

#include "AutoMode.h"
#include "words.h"
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
               int steps,
               const std::string &answer) {
    std::unordered_map<std::string, uint8_t> guessFeedback;
    size_t guess_count = 1;

    std::vector<size_t> sol_indices;
    sol_indices.reserve(solutions.size());
    for (const auto &s : solutions) {
        int idx = Feedback::get_valid_word_index(s);
        if (idx != -1) sol_indices.push_back(idx);
    }

    while (sol_indices.size() > 1) {
        int k = steps;
        if (7 - guess_count < steps) {
            k = 7 - guess_count;
        }

        auto [best_guess, best_entropy] = entropy.get_best_guess(
            guesses, solutions, k, display);

        guesses.erase(std::remove(guesses.begin(), guesses.end(), best_guess),
                            guesses.end());

        display.showOutput("Next Guess: " + best_guess + " | Expected Entropy (over " + std::to_string(k) + " steps): " + std::to_string(best_entropy));

        // Get feedback for actual answer
        uint8_t fb_encoded = feedback.get_feedback(best_guess, answer);

        int best_guess_all_idx = Feedback::get_all_word_index(best_guess);
        sol_indices = feedback.get_new_solutions(sol_indices, best_guess_all_idx, fb_encoded);

        // Update solutions string vector for display/entropy next call
        solutions.clear();
        for (size_t idx : sol_indices) {
            solutions.push_back(VALID_WORDS[idx]);
        }

        guess_count ++;
    }

    if (!sol_indices.empty()) {
        display.showOutput("Solution found in " + std::to_string(guess_count) + " guesses: " + VALID_WORDS[sol_indices[0]]);
        return guess_count;
    } else {
        display.showOutput("No solution found.\n");
    }
    return -1;
}