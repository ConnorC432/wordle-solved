//
// Created by connor on 12/09/2025.
//

#ifndef WORDLE_SOLVED_AUTOMODE_H
#define WORDLE_SOLVED_AUTOMODE_H
#pragma once
#include "Feedback.h"
#include "Display.h"
#include "Entropy.h"


class AutoMode {
private:
    Display &display;
    Feedback &feedback;
    Entropy entropy;

public:
    AutoMode(Display &display, Feedback &feedback_instance);

    int run(std::vector<std::string> all_solutions,
             std::vector<std::string> solutions,
             const std::string &answer);
};


#endif //WORDLE_SOLVED_AUTOMODE_H