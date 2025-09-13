//
// Created by connor on 12/09/2025.
//

#include <algorithm>
#include <csignal>
#include <iostream>
#include <ostream>

#include "AutoMode.h"
#include "Feedback.h"
#include "Display.h"
#include "InteractiveMode.h"
#include "words.h"


int main(int argc, char *argv[]) {
    Display display;
    Feedback feedback(display);
    bool silent = false;

    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--silent") {
            Display::setSilent(true);
            silent = true;
        }
    }

    std::vector<std::string> all_solutions = get_all_solutions();
    std::vector<std::string> valid_solutions = get_valid_solutions();

    feedback.precache_feedback(all_solutions);
    Entropy::precache_log(all_solutions.size());

    if (argc > 1) {
        std::string answer = argv[1];

        for (char &c : answer) c = tolower(c);

        if (std::find(valid_solutions.begin(), valid_solutions.end(), answer) == valid_solutions.end()) {
            display.showOutput("Error: provided answer is not in solution list.\n");
            return 1;
        }

        AutoMode automode(display, feedback);
        int guesses = automode.run(all_solutions, valid_solutions, answer);

        if (guesses == -1) {
            return 1;
        }

        if (silent) {
            std::cout << guesses << std::endl;
        }
    } else {
        display.showOutput("Interactive Mode Starting");
        InteractiveMode interactivemode(display, feedback);
        interactivemode.run(all_solutions, valid_solutions);
    }
}