//
// Created by connor on 12/09/2025.
//

#include "Entropy.h"
#include "ThreadPool.h"
#include "LogTable.h"
#include "words.h"
#include <algorithm>
#include <array>
#include <future>
#include <vector>
#include <string>
#include <cmath>
#include <thread>
#include <atomic>
#include <iostream>

Entropy::Entropy(const Feedback &fbCache) : cache(fbCache) {}

// Single-step Entropies
std::vector<double> Entropy::get_entropy(
    const std::vector<size_t> &guesses,
    const std::vector<size_t> &solutions
) const {
    std::vector<double> results(guesses.size());
    size_t total = solutions.size();
    if (total == 0) return results;

    double log2_total = std::log2(total);

    for (size_t i = 0; i < guesses.size(); ++i) {
        size_t guess_idx = guesses[i];
        std::array<size_t, 243> counts{};
        counts.fill(0);

        for (size_t sol_idx : solutions) {
            counts[cache.get_feedback_cached(guess_idx, sol_idx)]++;
        }

        double entropy = 0.0;
        for (size_t count : counts) {
            if (count == 0) continue;
            double p = static_cast<double>(count) / total;
            double logp = (count < 16001) ? LOG2_TABLE[count] - log2_total : std::log2(p);
            entropy -= p * logp;
        }
        results[i] = entropy;
    }

    return results;
}

// Get N-Step Entropies
std::vector<std::pair<size_t, double>> Entropy::get_n_step_entropy(
    const std::vector<size_t> &guesses,
    const std::vector<size_t> &solutions,
    int k,
    std::atomic<size_t> &progress,
    bool top_level
) const {
    std::vector<std::pair<size_t, double>> results(guesses.size());
    if (solutions.empty()) return results;

    auto compute_for_guess = [&](size_t i) {
        size_t guess_idx = guesses[i];
        size_t total = solutions.size();

        std::array<size_t, 243> counts{};
        counts.fill(0);
        for (size_t sol_idx : solutions) {
            counts[cache.get_feedback_cached(guess_idx, sol_idx)]++;
        }

        // 1. Single-step entropy
        double H_current = 0.0;
        double log2_total = std::log2(total);
        for (size_t count : counts) {
            if (count == 0) continue;
            double p = static_cast<double>(count) / total;
            double logp = (count < 16001) ? LOG2_TABLE[count] - log2_total : std::log2(p);
            H_current -= p * logp;
        }

        // 2. Weighted max entropy for next step
        double H_next = 0.0;
        if (k > 1 && total > 1) {
            for (size_t fb = 0; fb < 243; ++fb) {
                size_t count = counts[fb];
                if (count == 0) continue;

                std::vector<size_t> subset;
                subset.reserve(count);
                for (size_t sol_idx : solutions) {
                    if (cache.get_feedback_cached(guess_idx, sol_idx) == fb)
                        subset.push_back(sol_idx);
                }

                double max_branch_entropy = 0.0;
                if (count > 1) {
                    if (k > 2) {
                        auto next_entropies = get_n_step_entropy(guesses, subset, k - 1, progress, false);
                        for (auto &pair : next_entropies)
                            if (pair.second > max_branch_entropy) max_branch_entropy = pair.second;
                    } else {
                        auto entropies = get_entropy(guesses, subset);
                        for (double e : entropies)
                            if (e > max_branch_entropy) max_branch_entropy = e;
                    }
                }

                H_next += (static_cast<double>(count) / total) * max_branch_entropy;
            }
        }

        results[i] = {guess_idx, H_current + H_next};
        if (top_level) progress.fetch_add(1, std::memory_order_relaxed);
    };

    if (top_level) {
        ThreadPool pool(std::thread::hardware_concurrency());
        std::vector<std::future<void>> futures;
        futures.reserve(guesses.size());
        for (size_t i = 0; i < guesses.size(); ++i)
            futures.push_back(pool.enqueue([&, i]() { compute_for_guess(i); }));
        for (auto &f : futures) f.get();
    } else {
        for (size_t i = 0; i < guesses.size(); ++i)
            compute_for_guess(i);
    }

    return results;
}

std::pair<std::string, double> Entropy::get_best_guess(
    const std::vector<std::string> &guesses,
    const std::vector<std::string> &solutions,
    int k,
    Display &display
) const {
    // Standard Wordle opening (optional optimization)
    if (solutions.size() == VALID_WORD_COUNT && std::find(guesses.begin(), guesses.end(), "slate") != guesses.end()) {
        return {"slate", 0.0};
    }

    // Convert strings to indices once
    std::vector<size_t> guess_indices;
    guess_indices.reserve(guesses.size());
    for (const auto &g : guesses) {
        int idx = Feedback::get_all_word_index(g);
        if (idx != -1) guess_indices.push_back(idx);
    }

    std::vector<size_t> sol_indices;
    sol_indices.reserve(solutions.size());
    for (const auto &s : solutions) {
        int idx = Feedback::get_valid_word_index(s);
        if (idx != -1) sol_indices.push_back(idx);
    }

    display.showProgress("Calculating Best Guess", 0, guess_indices.size());
    std::atomic<size_t> progress(0);
    bool done = false;

    std::thread progress_thread([&]() {
        while (!done) {
            display.showProgress("Calculating Best Guess", progress.load(), guess_indices.size());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    auto results = get_n_step_entropy(guess_indices, sol_indices, k, progress, true);
    done = true;
    progress_thread.join();

    auto max_it = std::max_element(results.begin(), results.end(),
        [](const auto &a, const auto &b) { return a.second < b.second; });

    return {ALL_WORDS[max_it->first], max_it->second};
}
