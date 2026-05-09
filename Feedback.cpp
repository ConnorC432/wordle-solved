//
// Created by connor on 12/09/2025.
//

#include "Feedback.h"
#include "words.h"
#include "FeedbackTable.h"
#include <atomic>
#include <thread>
#include <cstdint>
#include <unordered_map>
#include <vector>

std::unordered_map<std::string, size_t> Feedback::all_word_index;
std::unordered_map<std::string, size_t> Feedback::valid_word_index;

// Encode Feedback to Base 3
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

// TODO: Separate feedback cache table by Base 3 Feedback int, optimise get cache function
uint8_t Feedback::get_feedback_cached(const std::string &guess, const std::string &solution) const {
    auto it_guess = all_word_index.find(guess);
    if (it_guess == all_word_index.end()) return 0;
    size_t idx = it_guess->second;

    auto it_sol = valid_word_index.find(solution);
    if (it_sol == valid_word_index.end()) return 0;
    size_t sol_idx = it_sol->second;

    return FEEDBACK_TABLE[idx * VALID_WORD_COUNT + sol_idx];
}

// Cache Feedback with Thread Pool
void Feedback::precache_indices() {
    if (all_word_index.empty()) {
        for (size_t i = 0; i < ALL_WORD_COUNT; ++i)
            all_word_index[ALL_WORDS[i]] = i;
    }
    if (valid_word_index.empty()) {
        for (size_t i = 0; i < VALID_WORD_COUNT; ++i)
            valid_word_index[VALID_WORDS[i]] = i;
    }
}

// Update Solutions
std::vector<std::string> Feedback::get_new_solutions(
    const std::vector<std::string> &solutions,
    const std::string &guess,
    uint8_t feedback
) const {
    std::vector<std::string> new_solutions;

    for (const auto &sol : solutions) {
        if (get_feedback(guess, sol) == feedback)  // compute directly
            new_solutions.push_back(sol);
    }

    return new_solutions;
}