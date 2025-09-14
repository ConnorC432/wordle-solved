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


struct Options {
    int k = 1;
    bool fast = false;
    bool silent = false;
    std::string answer;
};

Options parse_args(int argc, char *argv[], Display &display) {
    Options opts;

    // First non "-" arg is treated as the answer
    if (argc > 1 && argv[1][0] != '-') {
        opts.answer = argv[1];
    }

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-f") {
            opts.fast = true;
        }
        else if (arg == "-s") {
            Display::setSilent(true);
            opts.silent = true;
        }
        else if (arg == "-k") {
            if (i + 1 < argc) {
                int parsed_k = std::stoi(argv[++i]);
                if (parsed_k >= 1 && parsed_k <= 6) {
                    opts.k = parsed_k;
                } else {
                    display.showOutput("Warning: -k must be between 1 and 6. Using the default value of 1.\n");
                }
            } else {
                display.showOutput("Error: -k flag requires a number.\n");
                exit(1);
            }
        }

        else if (arg.size() > 2 && arg[0] == '-') {
            for (size_t j = 1; j < arg.size(); j++) {
                char flag = arg[j];
                if (flag == 'f') {
                    opts.fast = true;
                }
                else if (flag == 's') {
                    Display::setSilent(true);
                    opts.silent = true;
                }
                else if (flag == 'k') {
                    if (i + 1 < argc) {
                        int parsed_k = std::stoi(argv[++i]);
                        if (parsed_k >= 1 && parsed_k <= 6) {
                            opts.k = parsed_k;
                        } else {
                            display.showOutput("Warning: -k must be between 1 and 6. Using default value 1.\n");
                        }
                    } else {
                        display.showOutput("Error: -k flag requires a number.\n");
                        exit(1);
                    }
                }
                else {
                    display.showOutput(std::string("Unknown flag: -") + flag + "\n");
                }
            }
        }
    }

    return opts;
}

int main(int argc, char *argv[]) {
    Display display;
    Feedback feedback(display);

    Options opts = parse_args(argc, argv, display);

    std::vector<std::string> guesses;
    if (opts.fast) {
        guesses = get_valid_solutions();
    } else {
        guesses = get_all_solutions();
    }

    std::vector<std::string> answers = get_valid_solutions();

    feedback.precache_feedback(guesses);
    Entropy::precache_log(guesses.size(), display);

    if (!opts.answer.empty()) {
        std::string answer = opts.answer;
        for (char &c : answer) c = tolower(c);

        if (std::find(answers.begin(), answers.end(), answer) == answers.end()) {
            display.showOutput("Error: provided answer is not in solution list.\n");
            return 1;
        }

        AutoMode automode(display, feedback);
        int guess_count = automode.run(guesses, answers, opts.k, answer);

        if (guess_count == -1) {
            return 1;
        }

        if (opts.silent) {
            std::cout << guess_count << std::endl;
        }
    } else {
        display.showOutput("Interactive Mode Starting");
        InteractiveMode interactivemode(display, feedback);
        interactivemode.run(guesses, answers, opts.k);
    }
}