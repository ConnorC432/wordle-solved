//
// Created by connor on 12/09/2025.
//

#include "Display.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <vector>

bool Display::global_silent = false;

void Display::showProgress(const std::string& title, size_t workDone, size_t totalWork) {
    if (global_silent) return;

    // Initialize progress if not started
    if (!progress.started || totalWork != progress.totalWork) {
        progress.totalWork = totalWork;
        progress.workDone = workDone;
        progress.startTime = std::chrono::steady_clock::now();
        progress.started = true;
    } else {
        progress.workDone = workDone;
    }

    displayProgressBar(title);

    if (workDone >= totalWork)
        progress.started = false; // Reset for next progress bar
}

void Display::displayProgressBar(const std::string& title) const {
    if (global_silent) return;

    using namespace std::chrono;
    const int barWidth = 50;

    // Calculate progress
    double progressPercent = static_cast<double>(progress.workDone) / progress.totalWork;
    int pos = static_cast<int>(barWidth * progressPercent);

    // Time calculations
    auto now = steady_clock::now();
    auto elapsed = duration_cast<seconds>(now - progress.startTime).count();
    double speed = elapsed > 0 ? static_cast<double>(progress.workDone) / elapsed : 0;

    // Build bar
    std::cout << "\r" << std::setw(30) << std::left << title << " [";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] ";

    // Percentage
    std::cout << std::setw(6) << std::right << std::fixed << std::setprecision(1) << (progressPercent * 100.0) << "% ";

    // Speed
    std::cout << std::setw(6) << std::fixed << std::setprecision(2) << speed << "it/s ";

    // Elapsed time
    int minutes = elapsed / 60;
    int seconds = elapsed % 60;
    std::cout << "[" << minutes << "m " << seconds << "s]";

    std::cout.flush();

    if (progress.workDone >= progress.totalWork)
        std::cout << std::endl;
}

void Display::showGuesses(
    const std::vector<std::pair<std::string, uint8_t>>& guessFeedback,
    const std::string& currentGuess) {
    if (global_silent) return;

    std::cout << "\n";
    for (const auto& [guess, fB] : guessFeedback) {
        if (guess == currentGuess) continue;

        std::cout << " ";

        int temp = fB;
        std::vector<int> fbVector(guess.size());

        for (int i = guess.size() - 1; i >= 0; --i) {
            fbVector[i] = temp % 3;
            temp /= 3;
        }

        for (size_t i = 0; i < guess.size(); ++i) {
            char letter = guess[i];

            if (fbVector[i] == 2)
                std::cout << "\033[1;42;30m " << letter << " \033[0m"; // Green
            else if (fbVector[i] == 1)
                std::cout << "\033[1;43;30m " << letter << " \033[0m"; // Yellow
            else
                std::cout << "\033[1;100;97m " << letter << " \033[0m"; // Gray
        }
        std::cout << "\n";
    }

    if (!currentGuess.empty()) {
        std::cout << " " << "\033[1;100;97m";
        for (size_t i = 0; i < currentGuess.size(); ++i) {
            std::cout << " " << currentGuess[i] << " ";
        }
        std::cout << "\033[0m\n\n";
    }
}

void Display::showOutput(const std::string& output) {
    if (global_silent) return;
    std::cout << output << std::endl;
}

void Display::clearDisplay() {
    if (global_silent) return;

#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}