//
// Created by connor on 12/09/2025.
//

#include "Entropy.h"
#include "ThreadPool.h"
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

// Logarithm Cache
std::vector<double> Entropy::log_cache;

void Entropy::precache_log(size_t max_size) {
    log_cache.resize(max_size + 1);
    for (size_t i = 1; i <= max_size; ++i) {
        double p = static_cast<double>(i) / max_size;
        log_cache[i] = std::log2(p);
    }
}

// Feedback Count
std::array<size_t, 243> Entropy::get_feedback_count(
    const std::string &guess,
    const std::vector<std::string> &solutions,
    const std::vector<size_t> &indices
) const {
    std::array<size_t, 243> counts{};
    counts.fill(0);

    for (size_t idx : indices) {
        uint8_t db = cache.get_feedback_cached(guess, solutions[idx]);
    }

    return counts;
}

// Single-step Entropies
std::vector<std::pair<std::string, double>> Entropy::get_entropy(
    const std::vector<std::string> &guesses,
    const std::vector<std::string> &solutions,
    const std::vector<size_t> &indices
) const {
    std::vector<std::pair<std::string, double>> results;
    results.reserve(guesses.size());

    size_t total = indices.size();
    if (total == 0) return results;

    for (const auto &guess : guesses) {
        std::array<size_t, 243> counts{};
        for (size_t idx : indices) {
            uint8_t fb = cache.get_feedback_cached(guess, solutions[idx]);
            counts[fb]++;
        }

        double entropy = 0.0;
        for (size_t count : counts) {
            if (count == 0) continue;
            double p = static_cast<double>(count) / total;
            double logp = (count < log_cache.size()) ? log_cache[count] - std::log2(total) : std::log2(p);
            entropy -= p * logp;
        }

        results.emplace_back(guess, entropy);
    }

    return results;
}

// Get Next Step Entropies
std::vector<std::pair<std::string, double>> Entropy::get_next_entropy(
    const std::vector<std::pair<std::string, double>> &entropies,
    const std::vector<std::string> &guesses,
    const std::vector<std::string> &solutions
) const {
    std::vector<std::pair<std::string, double>> result;
    result.reserve(entropies.size());

    std::vector<size_t> indices(solutions.size());
    for (size_t i = 0; i < solutions.size(); ++i) {
        indices[i] = i;
    }

    for (const auto &[first_guess, _] : entropies) {
        // Partition solutions by feedback pattern
        std::array<std::vector<size_t>, 243> partitions;
        for (size_t i = 0; i < solutions.size(); ++i) {
            partitions[i].reserve(solutions.size() / 243 + 1);
        }

        for (size_t idx : indices) {
            uint8_t fb = cache.get_feedback_cached(first_guess, solutions[idx]);
            partitions[fb].push_back(idx);
        }

        std::vector<std::pair<std::string, double>> conditional_entropies;
        conditional_entropies.reserve(guesses.size());

        for (size_t fb = 0; fb < partitions.size(); ++fb) {
            const auto &subset = partitions[fb];
            if (subset.empty()) continue;

            auto sub_entropies = get_entropy(guesses, solutions, subset);

            double weight = static_cast<double>(subset.size()) / solutions.size();
            for (size_t i = 0; i < sub_entropies.size(); i++) {
                if (conditional_entropies.size() < sub_entropies.size())
                    conditional_entropies.emplace_back(sub_entropies[i].first, 0.0);
                conditional_entropies[i].second += weight * sub_entropies[i].second;
            }
        }

        result.emplace_back(first_guess, 0.0);
    }

    return result;
}

// Get N-Step Entropies
std::vector<std::pair<std::string, double>> Entropy::get_n_step_entropy(
    const std::vector<std::string> &guesses,
    const std::vector<std::string> &solutions,
    int k,
    std::atomic<size_t> &progress,
    bool top_level
) const {
    std::vector<std::pair<std::string, double>> results(guesses.size());
    if (solutions.empty()) return results;

    auto compute_for_guess = [&](size_t i) {
        const std::string &guess = guesses[i];
        size_t total = solutions.size();

        // 1. Precompute feedbacks
        std::vector<uint8_t> feedbacks(total);
        std::array<size_t, 243> counts{};
        counts.fill(0);

        for (size_t j = 0; j < total; ++j) {
            uint8_t fb = cache.get_feedback_cached(guess, solutions[j]);
            feedbacks[j] = fb;
            counts[fb]++;
        }

        // 2. Single-step entropy
        double H_current = 0.0;
        for (size_t count : counts) {
            if (count == 0) continue;
            double p = static_cast<double>(count) / total;
            double logp = (count < log_cache.size()) ? log_cache[count] - std::log2(total) : std::log2(p);
            H_current -= p * logp;
        }

        // 3. Weighted max entropy for next step
        double H_next = 0.0;
        if (k > 1) {
            for (size_t fb = 0; fb < counts.size(); ++fb) {
                size_t count = counts[fb];
                if (count == 0) continue;

                // Collect subset indices
                std::vector<size_t> subset_indices;
                subset_indices.reserve(count);
                for (size_t j = 0; j < feedbacks.size(); ++j)
                    if (feedbacks[j] == fb)
                        subset_indices.push_back(j);

                if (subset_indices.empty()) continue;

                // Compute next-step entropy recursively
                double max_branch_entropy = 0.0;
                if (k > 2) {
                    auto next_entropies = get_n_step_entropy(guesses, solutions, k - 1, progress, false);
                    for (auto &[g, e] : next_entropies)
                        if (e > max_branch_entropy) max_branch_entropy = e;
                } else {
                    auto entropies = get_entropy(guesses, solutions, subset_indices);
                    for (auto &[g, e] : entropies)
                        if (e > max_branch_entropy) max_branch_entropy = e;
                }

                double weight = static_cast<double>(count) / total;
                H_next += weight * max_branch_entropy;
            }
        }

        results[i] = {guess, H_current + H_next};

        if (top_level)
            progress.fetch_add(1, std::memory_order_relaxed);
    };

    if (top_level) {
        // Use a single ThreadPool
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
    display.showProgress("Calculating Best Guess", 0, guesses.size());
    std::atomic<size_t> progress(0);
    bool done = false;

    // Progress thread
    std::thread progress_thread([&]() {
        while (!done) {
            display.showProgress("Calculating Best Guess", progress.load(), guesses.size());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        display.showProgress("Calculating Best Guess", guesses.size(), guesses.size());
    });

    auto entropies = get_n_step_entropy(guesses, solutions, k, progress, true);

    done = true;
    progress_thread.join();

    auto max_it = std::max_element(
        entropies.begin(),
        entropies.end(),
        [](const auto &a, const auto &b) { return a.second < b.second; }
    );

    return *max_it;
}