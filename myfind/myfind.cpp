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
#include <vector>
#include <istream>
#include <string>
#include <iostream>
#include <filesystem>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>


namespace fs = std::filesystem;


/* globale Variable fuer den Programmnamen */
char *program_name = nullptr;

void extractArguments(int argc, char *argv[], std::string &searchPath, std::vector<std::string> &targets, int c,
                      bool &isRecursive, bool &ignoresCase, int error);

bool contains(std::vector<std::string> &targets, const std::string &filename);

std::string toLower(std::string &filename);

/* Funktion print_usage() zur Ausgabe der usage Meldung */
void print_usage() {
    fprintf(stderr, "Usage: %s [-R] [-i] searchpath filename1 [filename2] ...[filenameN] \n", program_name);
    exit(EXIT_FAILURE);
}

/* main Funktion mit Argumentbehandlung */
int main(int argc, char *argv[]) {
    std::string searchPath;
    std::vector<std::string> targets = {};

    int c = 0;
    bool isRecursive = false;
    bool ignoresCase = false;
    int error = 0;

    program_name = argv[0];

    extractArguments(argc, argv, searchPath, targets, c, isRecursive, ignoresCase, error);

    int pid = getpid();

    DIR *dirp;
    struct dirent *direntp;


    if ((dirp = opendir(searchPath.c_str())) == nullptr) {
        perror("Failed to open directory");
        return 1;
    }


    while ((direntp = readdir(dirp)) != nullptr) {
        std::string originalFilename = direntp->d_name;
        if (originalFilename.empty() || originalFilename == "." || originalFilename == "..") {
            continue;
        }

        std::string filename01 = originalFilename;
        if (ignoresCase) {
            filename01 = toLower(originalFilename);
            for (auto &target: targets) {
                target = toLower(target);
            }
        }

        if (contains(targets, filename01)) {
            printf("%d: %s: %s\n", pid, originalFilename.c_str(), originalFilename.c_str());
        } else {
            // DO NOTHING
        }

    }
    while ((closedir(dirp) == -1) && (errno == EINTR)) { ; }

//    for (const auto &entry: fs::directory_iterator(searchPath)) {
//
//        std::cout << std::to_string(pid) + ":" + entry.path().filename().string() << std::endl;
//        printf("\n%d: %s", pid, entry.path().filename().c_str());
//    }


    return EXIT_SUCCESS;
}

std::string toLower(std::string &filename) {
    std::string copiedFilename = filename;
    std::transform(copiedFilename.begin(), copiedFilename.end(), copiedFilename.begin(),
                   [](unsigned char c) { return tolower(c); });

    return copiedFilename;
}

bool contains(std::vector<std::string> &targets, const std::string &filename) {
    return std::find(targets.begin(), targets.end(), filename) != targets.end();
}

void extractArguments(int argc, char *argv[], std::string &searchPath, std::vector<std::string> &targets, int c,
                      bool &isRecursive, bool &ignoresCase, int error) {
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

    for (auto & target : targets) {
        std::cout << target << std::endl;
    }
    std::cout << "\n" << std::endl;

}
