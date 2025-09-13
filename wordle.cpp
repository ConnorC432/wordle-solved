//
// Created by connor on 12/09/2025.
//

#include <algorithm>
#include <csignal>
#include "AutoMode.h"
#include "Feedback.h"
#include "Display.h"
#include "InteractiveMode.h"
#include "words.h"


int main(int argc, char *argv[]) {
    Display display;
    Feedback feedback(display);

    std::vector<std::string> all_solutions = get_all_solutions();
    std::vector<std::string> valid_solutions = get_valid_solutions();

    feedback.precache_feedback(all_solutions);
    Entropy::precache_log(all_solutions.size());

    if (argc > 1) {
        std::string answer = argv[1];
        for (char &c : answer) c = tolower(c);

        if (std::find(all_solutions.begin(), all_solutions.end(), answer) == all_solutions.end()) {
            display.showOutput("ErrorL provided answer is not in solution list.\n");
            return 1;
        }

        display.showOutput("Auto Mode Starting");
        AutoMode automode(display, feedback);
        automode.run(all_solutions, valid_solutions, answer);
    } else {
        display.showOutput("Interactive Mode Starting");
        InteractiveMode interactivemode(display, feedback);
        interactivemode.run(all_solutions, valid_solutions);
    }
}