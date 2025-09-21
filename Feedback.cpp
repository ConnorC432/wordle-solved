//
// Created by connor on 12/09/2025.
//

#include "Feedback.h"
#include <atomic>
#include <thread>
#include <cstdint>
#include <unordered_map>
#include <vector>

std::vector<std::vector<uint8_t>> Feedback::feedback_cache;
std::unordered_map<std::string, size_t> Feedback::word_index;

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
    auto it_guess = word_index.find(guess);
    if (it_guess == word_index.end()) return 0;
    size_t idx = it_guess->second;

    auto it_sol = word_index.find(solution);
    if (it_sol == word_index.end()) return 0;
    size_t sol_idx = it_sol->second;

    return feedback_cache[idx][sol_idx];
}

// Cache Feedback with Thread Pool
void Feedback::precache_feedback(const std::vector<std::string> &all_solutions) {
    size_t n_words = all_solutions.size();

    // Build word_index for fast lookup
    word_index.clear();
    for (size_t i = 0; i < n_words; ++i)
        word_index[all_solutions[i]] = i;

    feedback_cache.assign(n_words, std::vector<uint8_t>(n_words, 0));
    std::atomic<size_t> progress(0);

    size_t n_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;

    std::atomic<bool> done(false);
    auto start_time = std::chrono::high_resolution_clock::now();

    // Progress Bar Thread
    std::thread progress_thread([&] {
        while (!done) {
            {
                std::lock_guard<std::mutex> lock(display.getMutex());
                display.showProgress("Caching Feedback", progress.load(), n_words);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    // Worker Threads
    auto worker = [&](size_t start, size_t end) {
        for (size_t i = start; i < end; ++i) {
            for (size_t j = 0; j < n_words; ++j)
                feedback_cache[i][j] = get_feedback(all_solutions[i], all_solutions[j]);
            progress.fetch_add(1, std::memory_order_relaxed);
        }
    };

    // Split Work into Chunks
    size_t chunk_size = (n_words + n_threads - 1) / n_threads;
    for (size_t i = 0; i < n_threads; ++i) {
        size_t start = i * chunk_size;
        size_t end = std::min((start + chunk_size), n_words);
        threads.emplace_back(worker, start, end);
    }

    for (auto &t : threads) t.join();
    done = true;
    progress_thread.join();

    std::lock_guard<std::mutex> lock(display.getMutex());
    display.showProgress("Caching Feedback", n_words, n_words);
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