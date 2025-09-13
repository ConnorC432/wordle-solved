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
    static std::vector<std::vector<uint8_t>> feedback_cache;
    static std::unordered_map<std::string, size_t> word_index;
    Display &display;

public:
    Feedback(Display &display) : display(display) {}

    // Encode Feedback to Base 3
    uint8_t get_feedback(const std::string &guess, const std::string &solution) const;

    // Cached Feedback
    uint8_t get_feedback_cached(const std::string &guess, const std::string &solution) const;

    // Cache Feedback with Thread Pool
    void precache_feedback(const std::vector<std::string> &all_solutions);

    const std::vector<std::vector<uint8_t>> &get_cache() const {return feedback_cache;};

    // Update Solutions
    std::vector<std::string> get_new_solutions(
        const std::vector<std::string> &solutions,
        const std::string &guess,
        uint8_t feedback
    ) const;
};


#endif //WORDLE_SOLVED_FEEDBACK_H