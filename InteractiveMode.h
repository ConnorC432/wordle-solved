//
// Created by connor on 12/09/2025.
//

#ifndef WORDLE_SOLVED_INTERACTIVEMODE_H
#define WORDLE_SOLVED_INTERACTIVEMODE_H
#pragma once
#include <vector>
#include <string>
#include "Display.h"
#include "Entropy.h"
#include "Feedback.h"


class InteractiveMode {
private:
    Display &display;
    Feedback &feedback;
    Entropy entropy;

public:
    InteractiveMode(Display &display, Feedback &feedback_instance);

    void run(std::vector<std::string> all_solutions,
             std::vector<std::string> solutions);
};


#endif //WORDLE_SOLVED_INTERACTIVEMODE_H