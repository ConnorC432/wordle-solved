#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <algorithm>
#include "words.h"

// Generate feedback string (G=green, Y=yellow, B=black)
std::string get_feedback(const std::string &guess, const std::string &solution) {
    std::string pattern = "BBBBB";
    std::string chars = solution;

    // Greens
    for (int i = 0; i < 5; ++i) {
        if (guess[i] == solution[i]) {
            pattern[i] = 'G';
            chars[i] = '*';
        }
    }

    // Yellows
    for (int i = 0; i < 5; ++i) {
        if (pattern[i] == 'B') {
            auto pos = chars.find(guess[i]);
            if (pos != std::string::npos) {
                pattern[i] = 'Y';
                chars[pos] = '*';
            }
        }
    }
    return pattern;
}

// Calculate entropy of a guess
double calculate_entropy(const std::string &guess, const std::vector<std::string> &solutions) {
    std::map<std::string, int> pattern_count;
    for (const auto &sol : solutions) {
        std::string pattern = get_feedback(guess, sol);
        pattern_count[pattern]++;
    }

    double entropy = 0.0;
    double total = static_cast<double>(solutions.size());
    for (const auto &p : pattern_count) {
        double prob = p.second / total;
        entropy -= prob * std::log2(prob);
    }
    return entropy;
}

// Get the best guess based on maximum entropy
std::pair<std::string, double> get_best_guess(const std::vector<std::string> &solutions,
                                              const std::vector<std::string> &all_solutions) {
    std::string best_guess;
    double best_entropy = -1.0;

    for (const auto &guess : all_solutions) {
        double entropy = calculate_entropy(guess, solutions);
        if (entropy > best_entropy) {
            best_entropy = entropy;
            best_guess = guess;
        }
    }

    return {best_guess, best_entropy};
}

// Filter solutions based on feedback
std::vector<std::string> get_new_solutions(const std::vector<std::string> &solutions,
                                           const std::string &guess,
                                           const std::string &feedback) {
    std::vector<std::string> new_solutions;
    for (const auto &s : solutions) {
        if (get_feedback(guess, s) == feedback) {
            new_solutions.push_back(s);
        }
    }

    if (new_solutions.empty()) {
        std::cout << "Warning: No remaining solutions match this feedback.\n";
        return solutions; // keep old solutions instead of emptying the list
    }

    double info_gain = std::log2(solutions.size()) - std::log2(new_solutions.size());
    std::cout << "\033[1mRemoved:\033[0m " << solutions.size() - new_solutions.size()
              << " | Information Gain: " << info_gain << " bits\n";

    return new_solutions;
}

// Color the word for terminal output
std::string colour_string(const std::string &word, const std::string &feedback) {
    std::string result;
    for (size_t i = 0; i < word.size(); ++i) {
        char letter = toupper(word[i]);
        char fb = feedback[i];
        switch (fb) {
            case 'G':
                result += "\033[1;42;30m " + std::string(1, letter) + " \033[0m";
                break;
            case 'Y':
                result += "\033[1;43;30m " + std::string(1, letter) + " \033[0m";
                break;
            case 'B':
                result += "\033[1;100;97m " + std::string(1, letter) + " \033[0m";
                break;
            default:
                result += letter;
        }
    }
    return result;
}

void display_guesses(const std::vector<std::string>& guesses) {
    for (const auto &guess : guesses) {
        std::cout << "\n " << guess;
    }
}

void display_current(const std::string& best_guess, double entropy) {
    std::cout << "\n " << colour_string(best_guess, "BBBBB")
              << "\n\n" << "\033[1mExpected Entropy:\033[0m " << entropy << " bits\n";

    std::cout << "\033[1mEnter feedback for this guess\033[0m ("
              << "\033[1;42;30m G \033[0m/"
              << "\033[1;43;30m Y \033[0m/"
              << "\033[1;100;97m B \033[0m/"
              << "\033[1mN\033[0m): "
              << "\n";
}

int main() {
    std::vector<std::string> all_solutions = get_all_solutions();
    if (all_solutions.empty()) {
        std::cerr << "Error: words.txt not found or empty.\n";
        return 1;
    }

    std::vector<std::string> solutions = all_solutions;
    std::vector<std::string> guesses;

    while (solutions.size() > 1) {
        auto result = get_best_guess(solutions, all_solutions);
        std::string best_guess = result.first;
        double best_entropy = result.second;

        std::cout << "\033[2J\033[1;1H";

        // Show previous guesses
        display_guesses(guesses);

        // Show current suggestion
        display_current(best_guess, best_entropy);

        std::string feedback;
        while (true) {
            std::cin >> feedback;

            // Convert to uppercase
            for (char &c : feedback) c = toupper(c);

            // Remove invalid guess
            if (feedback.find('N') != std::string::npos) {
                all_solutions.erase(std::remove(all_solutions.begin(), all_solutions.end(), best_guess),
                                    all_solutions.end());
                break; // go to next guess
            }

            // Apply feedback
            auto new_solutions = get_new_solutions(solutions, best_guess, feedback);
            if (new_solutions == solutions) {
                std::cout << "Invalid feedback! Please enter correct feedback.\n";
            } else {
                solutions = new_solutions;
                guesses.push_back(colour_string(best_guess, feedback));
                break;
            }
        }
    }

    if (!solutions.empty()) {
        for (const auto &guess : guesses)
            std::cout << " " << guess << "\n";

        std::cout << " " << colour_string(solutions[0], "GGGGG") << "\n";

        std::cout << "Solution found in " << guesses.size() + 1 << " guesses: "
                  << solutions[0] << "\n";
    } else {
        std::cout << "No solution could be found.\n";
    }

    return 0;
}
