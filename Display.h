//
// Created by connor on 12/09/2025.
//

#ifndef WORDLE_SOLVED_DISPLAY_H
#define WORDLE_SOLVED_DISPLAY_H
#pragma once
#include <string>
#include <chrono>
#include <mutex>
#include <vector>

class Display {
private:
    struct ProgressData {
        size_t totalWork;
        size_t workDone;
        std::chrono::time_point<std::chrono::steady_clock> startTime;
        bool started;
    };

    ProgressData progress;
    std::mutex cout_mutex;
    static bool global_silent;

    void displayProgressBar(const std::string& title) const;

public:
    Display() {progress.started = false;}
    Display(bool silent) {progress.started = false; global_silent = silent;}

    // Start or update a progress bar
    void showProgress(const std::string& title, size_t workDone, size_t totalWork);

    void showGuesses(
        const std::vector<std::pair<std::string, uint8_t>>& guessFeedback,
        const std::string& currentGuess= ""
        );

    void showOutput(const std::string& output);

    void clearDisplay();

    std::mutex& getMutex() {return cout_mutex;};

    static void setSilent(bool silent) {global_silent = silent;}
    static bool isSilent() {return global_silent;}
};


#endif //WORDLE_SOLVED_DISPLAY_H