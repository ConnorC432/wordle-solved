//
// Created by connor on 12/09/2025.
//

#include "InteractiveMode.h"
#include <algorithm>
#include <vector>
#include <string>
#include <iostream>

InteractiveMode::InteractiveMode(Display &display, Feedback &feedback_instance)
    : display(display),
      feedback(feedback_instance),
      entropy(feedback) {}

void InteractiveMode::run(std::vector<std::string>  guesses,
                      std::vector<std::string>  solutions) {
    std::vector<std::pair<std::string, uint8_t>> guessFeedback;
    size_t guess_count = 0;

    while (solutions.size() > 1) {
        auto [best_guess, best_entropy] = entropy.get_best_guess(
            guesses, solutions, 2, display);

        guesses.erase(std::remove(guesses.begin(), guesses.end(), best_guess),
                            guesses.end());

        display.clearDisplay();
        display.showGuesses(guessFeedback, best_guess);
        display.showOutput("Next Guess: " + best_guess + " | Expected Entropy: " + std::to_string(best_entropy));

        std::string fb_input;
        while (true) {
            std::cin >> fb_input;

            // Remove Invalid Guess
            if (fb_input.find('N') != std::string::npos || fb_input.find('n') != std::string::npos) {
                guesses.erase(std::remove(guesses.begin(), guesses.end(), best_guess), guesses.end());
                break;
            }

            // Validate input length and characters
            if (fb_input.length() != 5 || fb_input.find_first_not_of("GYBgyb") != std::string::npos) {
                std::cout << "Invalid feedback! Must be 5 letters using G/Y/B/ or N for an invalid guess.\n";
                continue;
            }

            // Encode feedback to Base 3
            uint8_t fb_encoded = 0;
            for (char c : fb_input) {
                uint8_t code = 0;
                switch (toupper(c)) {
                    case 'G': code = 2; break;
                    case 'Y': code = 1; break;
                    case 'B': code = 0; break;
                    default: code = 0; break;
                }
                fb_encoded = fb_encoded * 3 + code;
            }

            auto new_solutions = feedback.get_new_solutions(solutions, best_guess, fb_encoded);

            if (new_solutions.empty()) {
                std::cout << "Invalid Feedback! Please enter correct feedback. \n";
            } else {
                solutions = new_solutions;
                guessFeedback.push_back({best_guess, fb_encoded});
                guess_count++;
                break;
            }
        }
    }

    if (!solutions.empty()) {
        guessFeedback.push_back({solutions[0], 242});

        display.clearDisplay();
        display.showGuesses(guessFeedback);
        display.showOutput("\nSolution found in " + std::to_string(guess_count + 1) + " guesses: " + solutions[0]);
    } else {
        display.showOutput("No solution found.\n");
    }
}