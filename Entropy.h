//
// Created by connor on 12/09/2025.
//

#ifndef WORDLE_SOLVED_ENTROPY_H
#define WORDLE_SOLVED_ENTROPY_H
#pragma once
#include "Feedback.h"
#include "ThreadPool.h"
#include <array>
#include <atomic>
#include <vector>
#include <string>
#include <utility>
#include <memory>


class Entropy {
private:
    const Feedback &cache;
    mutable std::unique_ptr<ThreadPool> pool;

    // Single-step Entropies
    std::vector<double> get_entropy(
        const std::vector<size_t> &guesses,
        const std::vector<size_t> &solutions
    ) const;

public:
    Entropy(const Feedback &fbCache);

    // N-step entropy
    std::vector<std::pair<size_t, double>> get_n_step_entropy(
        const std::vector<size_t> &guesses,
        const std::vector<size_t> &solutions,
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