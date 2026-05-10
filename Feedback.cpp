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

#include <algorithm>


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

int Feedback::get_all_word_index(const std::string &word) {
    auto it = std::lower_bound(ALL_WORDS, ALL_WORDS + ALL_WORD_COUNT, word);
    if (it != ALL_WORDS + ALL_WORD_COUNT && *it == word) {
        return std::distance(ALL_WORDS, it);
    }
    return -1;
}

int Feedback::get_valid_word_index(const std::string &word) {
    auto it = std::lower_bound(VALID_WORDS, VALID_WORDS + VALID_WORD_COUNT, word);
    if (it != VALID_WORDS + VALID_WORD_COUNT && *it == word) {
        return std::distance(VALID_WORDS, it);
    }
    return -1;
}

// Cache Feedback with Thread Pool
void Feedback::precache_indices() {
}

// Update Solutions
std::vector<size_t> Feedback::get_new_solutions(
    const std::vector<size_t> &solutions,
    size_t guess_idx,
    uint8_t feedback
) const {
    std::vector<size_t> new_solutions;
    new_solutions.reserve(solutions.size() / 3);

    for (size_t sol_idx : solutions) {
        if (get_feedback_cached(guess_idx, sol_idx) == feedback)
            new_solutions.push_back(sol_idx);
    }

    return new_solutions;
}