//
// Created by connor on 12/09/2025.
//

#ifndef WORDLE_SOLVED_FEEDBACK_H
#define WORDLE_SOLVED_FEEDBACK_H
#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "Display.h"


class Feedback {
private:
    Display &display;

public:
    Feedback(Display &display) : display(display) {}

    // Encode Feedback to Base 3
    uint8_t get_feedback(const std::string &guess, const std::string &solution) const;

    // Cached Feedback
    inline uint8_t get_feedback_cached(size_t guess_idx, size_t sol_idx) const {
        return FEEDBACK_TABLE[guess_idx * VALID_WORD_COUNT + sol_idx];
    }

    // Get index of word
    static int get_all_word_index(const std::string &word);
    static int get_valid_word_index(const std::string &word);

    // Update Solutions
    std::vector<size_t> get_new_solutions(
        const std::vector<size_t> &solutions,
        size_t guess_idx,
        uint8_t feedback
    ) const;
};


#endif //WORDLE_SOLVED_FEEDBACK_H