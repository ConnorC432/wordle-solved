//
// Created by connor on 12/09/2025.
//

#ifndef WORDLE_SOLVED_ENTROPY_H
#define WORDLE_SOLVED_ENTROPY_H
#pragma once
#include "Feedback.h"
#include <array>
#include <atomic>
#include <vector>
#include <string>
#include <utility>


class Entropy {
private:
    const Feedback &cache;

    // Feedback Count
    std::array<size_t, 243> get_feedback_count(
        const std::string &guess,
        const std::vector<std::string> &solutions,
        const std::vector<size_t> &indices
    ) const;

    // Single-step Entropies
    std::vector<std::pair<std::string, double>> get_entropy(
        const std::vector<std::string> &guesses,
        const std::vector<std::string> &solutions,
        const std::vector<size_t> &indices
    ) const;

    // Get Next Step Entropies
    std::vector<std::pair<std::string, double>> get_next_entropy(
        const std::vector<std::pair<std::string, double>> &entropies,
        const std::vector<std::string> &guesses,
        const std::vector<std::string> &solutions
    ) const;


public:
    Entropy(const Feedback &fbCache);

    static std::vector<double> log_cache;
    static void precache_log(size_t max_size);

    // N-step entropy
    std::vector<std::pair<std::string, double>> get_n_step_entropy(
        const std::vector<std::string> &solutions,
        const std::vector<std::string> &all_solutions,
        int k,
        std::atomic<size_t> &progress,
        bool top_level
    ) const;

    std::pair<std::string, double> get_best_guess(
        const std::vector<std::string> &guesses,
        const std::vector<std::string> &solutions,
        int k,
        Display &display
        ) const;
};


#endif //WORDLE_SOLVED_ENTROPY_H