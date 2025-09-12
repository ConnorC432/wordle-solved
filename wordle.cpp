#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <future>
#include <chrono>

#include "words.h"

std::unordered_map<std::string, std::unordered_map<std::string, std::string>> feedback_cache;
std::mutex cache_mutex;
std::atomic<size_t> cache_progress(0);
std::mutex cout_mutex;
std::atomic<size_t> two_step_progress(0);

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

// Precache feedback patterns for all word pairs
void cache_feedback_range(const std::vector<std::string> &all_solutions, size_t start, size_t end) {
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> local_cache;
    auto start_time = std::chrono::steady_clock::now();

    auto draw_bar = [](double fraction, size_t width = 40) {
        size_t pos = static_cast<size_t>(fraction * width);
        std::string bar = "[";
        for (size_t i = 0; i < width; ++i) {
            bar += (i < pos ? '=' : ' ');
        }
        bar += "]";
        return bar;
    };

    for (size_t i = start; i < end; ++i) {
        const auto &guess = all_solutions[i];
        for (const auto &solution : all_solutions) {
            local_cache[guess][solution] = get_feedback(guess, solution);
        }

        // Update progress
        cache_progress++;
        if (cache_progress % 50 == 0 || cache_progress == all_solutions.size()) {
            auto now = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration<double>(now - start_time).count();
            double fraction = cache_progress * 1.0 / all_solutions.size();

            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "\rCaching feedback:       " << draw_bar(fraction)
                      << " " << cache_progress << "/" << all_solutions.size()
                      << " | " << (cache_progress / elapsed) << " it/s " << std::flush;
        }
    }

    // Merge local cache into global cache
    std::lock_guard<std::mutex> lock(cache_mutex);
    for (auto &outer : local_cache) {
        feedback_cache[outer.first].insert(outer.second.begin(), outer.second.end());
    }
}

void precache_feedback(const std::vector<std::string> &all_solutions) {
    size_t n_threads = std::thread::hardware_concurrency();
    size_t n_words = all_solutions.size();
    size_t chunk_size = (n_words + n_threads - 1) / n_threads;

    std::vector<std::thread> threads;

    std::cout << "\n"; // space for bar

    for (size_t t = 0; t < n_threads; ++t) {
        size_t start = t * chunk_size;
        size_t end = std::min(start + chunk_size, n_words);
        threads.emplace_back(cache_feedback_range, std::cref(all_solutions), start, end);
    }

    for (auto &th : threads)
        th.join();

    std::cout << "\rCaching feedback:       [========================================] "
              << all_solutions.size() << "/" << all_solutions.size() << " | done        "
              << std::flush;
}

// Calculate entropy of a guess
double calculate_entropy(const std::string &guess, const std::vector<std::string> &solutions) {
    std::map<std::string, int> pattern_count;
    for (const auto &sol : solutions) {
        std::string pattern = feedback_cache[guess][sol];
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

// Calculate two-step entropy
double calculate_two_step_entropy(
    const std::string &guess,
    const std::vector<std::string> &solutions,
    const std::vector<std::string> &all_solutions)
{
    // Group solutions by feedback
    std::map<std::string, std::vector<std::string>> feedback_map;
    for (const auto &sol : solutions) {
        std::string feedback = feedback_cache[guess][sol];
        feedback_map[feedback].push_back(sol);
    }

    double total_solutions = static_cast<double>(solutions.size());

    // Get First Guess Entropy
    double first_entropy = 0.0;
    for (auto &entry : feedback_map) {
        double p = static_cast<double>(entry.second.size()) / total_solutions;
        first_entropy -= p * std::log2(p);  // Shannon entropy
    }

    // Get Weighted Second Guess Entropies
    double expected_second_entropy = 0.0;
    for (auto &entry : feedback_map) {
        const auto &filtered_solutions = entry.second;
        double p = static_cast<double>(filtered_solutions.size()) / total_solutions;

        double best_second_entropy = 0.0;
        for (const auto &second_guess : all_solutions) {
            double entropy = calculate_entropy(second_guess, filtered_solutions);
            if (entropy > best_second_entropy) {
                best_second_entropy = entropy;
            }
        }

        expected_second_entropy += p * best_second_entropy;
    }

    return first_entropy + expected_second_entropy;
}

std::pair<std::string, double> get_best_guess_two_step(
    const std::vector<std::string> &solutions,
    const std::vector<std::string> &all_solutions)
{
    std::string best_guess;
    double best_entropy = -1.0;

    std::atomic<size_t> progress(0);
    const size_t n_threads = std::thread::hardware_concurrency();
    std::atomic<size_t> index(0);
    std::mutex best_mutex;
    auto start_time = std::chrono::steady_clock::now();

    auto draw_bar = [](double fraction, size_t width = 40) {
        size_t pos = static_cast<size_t>(fraction * width);
        std::string bar = "[";
        for (size_t i = 0; i < width; ++i) {
            bar += (i < pos ? '=' : ' ');
        }
        bar += "]";
        return bar;
    };

    auto worker = [&]() {
        size_t i;
        while ((i = index.fetch_add(1)) < all_solutions.size()) {
            const auto &guess = all_solutions[i];
            double guess_entropy = calculate_two_step_entropy(guess, solutions, all_solutions);

            {
                std::lock_guard<std::mutex> lock(best_mutex);
                if (guess_entropy > best_entropy) {
                    best_entropy = guess_entropy;
                    best_guess = guess;
                }
            }

            size_t done = ++progress;
            if (done % 1 == 0 || done == all_solutions.size()) {
                auto now = std::chrono::steady_clock::now();
                double elapsed = std::chrono::duration<double>(now - start_time).count();
                double fraction = done * 1.0 / all_solutions.size();

                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "\rCalculating best guess: " << draw_bar(fraction)
                          << " " << done << "/" << all_solutions.size()
                          << " | " << (done / elapsed) << " it/s " << std::flush;
            }
        }
    };

    // Print initial empty progress bar
    std::cout << "\nCalculating best guess: [                                        ] 0/"
              << all_solutions.size() << " | 0 it/s   " << std::flush;

    std::vector<std::thread> threads;
    for (size_t t = 0; t < n_threads; ++t)
        threads.emplace_back(worker);

    for (auto &th : threads)
        th.join();

    std::cout << "\rCalculating best guess: [========================================] "
              << all_solutions.size() << "/" << all_solutions.size() << " | done"
              << std::flush;

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
    precache_feedback(all_solutions);

    std::vector<std::string> solutions = get_valid_solutions();
    std::vector<std::string> guesses;

    while (solutions.size() > 1) {
        auto result = get_best_guess_two_step(solutions, all_solutions);
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