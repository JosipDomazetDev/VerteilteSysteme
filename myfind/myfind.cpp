/**
 *
 * myfind.cpp
 * Verteilte Systeme
 */

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include <string>
#include <vector>
#include <istream>

/* globale Variable fuer den Programmnamen */
char *program_name = nullptr;

void extractArguments(int argc, char *argv[], std::string &searchPath, std::vector<std::string> &targets, int c,
                      bool isRecursive, bool ignoresCase, int error);

/* Funktion print_usage() zur Ausgabe der usage Meldung */
void print_usage() {
    fprintf(stderr, "Usage: %s [-R] [-i] searchpath filename1 [filename2] ...[filenameN] \n", program_name);
    exit(EXIT_FAILURE);
}

/* main Funktion mit Argumentbehandlung */
int main(int argc, char *argv[]) {
    std::string searchPath;
    std::vector<std::string> targets = {};

    int c= 0;
    bool isRecursive = false;
    bool ignoresCase = false;
    int error = 0;

    program_name = argv[0];

    extractArguments(argc, argv, searchPath, targets, c, isRecursive, ignoresCase, error);



    return EXIT_SUCCESS;
}

void extractArguments(int argc, char *argv[], std::string &searchPath, std::vector<std::string> &targets, int c,
                      bool isRecursive, bool ignoresCase, int error) {
    while ((c = getopt(argc, argv, "iR")) != EOF) {
        switch (c) {
            case 'i':        /* Option ohne Argument */
                ignoresCase = true;
                break;
            case 'R':                 /* Option mit Argument */
                isRecursive = true;
                break;
            case '?': /* unguelgtiges Argument */
                error = 1;
                break;
            default: /* unmoegliech */
                assert(0);
        }
    }
    if (error) /* Optionen fehlerhaft ? */
    {
        print_usage();
    }
    if ((argc < optind + 1)) /* falsche Anzahl an Optionen */
    {
        print_usage();
    }

    while (optind < argc) {

        /* aktuelles Argument: argv[optind] */
        if (searchPath.empty()) {
            searchPath = argv[optind];
        } else {
            targets.emplace_back(argv[optind]);
        }

        optind++;
    }


    std::cout << "=============== Used arguments ===============" << std::endl;
    std::cout << "isRecursive: " + std::to_string(isRecursive) << std::endl;
    std::cout << "ignoresCase: " + std::to_string(ignoresCase) << std::endl;
    std::cout << "searchPath: " + searchPath << std::endl;
    std::cout << "\nTargets: " << std::endl;

    for (unsigned int i = 0; i < targets.size(); i++) {
        std::cout << targets[i] << std::endl;
    }
}
