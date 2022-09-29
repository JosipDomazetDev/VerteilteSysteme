/**
 *
 * myfind.cpp
 * Verteilte Systeme
 */

#include <getopt.h>
#include <iostream>
#include <vector>
#include <istream>
#include <string>
#include <iostream>
#include <filesystem>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <cassert>

namespace fs = std::filesystem;


/* globale Variable fuer den Programmnamen */
char *program_name = nullptr;

void extractArguments(int argc, char *argv[], std::string &searchPath, std::vector<std::string> &targets, int c,
                      bool &isRecursive, bool &ignoresCase, int error);

bool contains(std::vector<std::string> &targets, const std::string &filename);

std::string toLower(std::string &filename);

void
handleCase(std::string &targets, bool ignoresCase, std::string &originalFilename, std::string &filename01);

void
traverseDirectory(const std::string &searchPath, std::string &targets, bool isRecursive, bool ignoresCase);

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


    for (auto &target: targets) {
        pid_t pid, wpid;
        int status;

        pid = fork();


        switch (pid) {
            case 1: /* error */
                fprintf(stderr, "myfork: error when forking child process\n");
                return EXIT_FAILURE;
            case 0: /* child process */
                traverseDirectory(searchPath, target, isRecursive, ignoresCase);
                return 3;
            default: /* parent */
                while ((wpid = wait(&status)) != pid) {
                    if (wpid != -1)
                        continue; /* different child process has terminated, continue waiting */

                    /* error waiting */
                    fprintf(stderr, "myfork: error when waiting for child process\n");
                    return EXIT_FAILURE;
                }
                /* check exit code of child after finishing */
                if (WIFEXITED(status)) /* child has finished normally with exit code WEXITSTATUS(status) */
                {
                   // printf("Child has finished normally, exit code: %d\n", WEXITSTATUS(status));
                } else /* child has finished with error */
                {
                    printf("Child process has finished with error or via signal\n");
                }
        }
    }




    //    for (const auto &entry: fs::directory_iterator(searchPath)) {
//
//        std::cout << std::to_string(pid) + ":" + entry.path().filename().string() << std::endl;
//        printf("\n%d: %s", pid, entry.path().filename().c_str());
//    }


    return EXIT_SUCCESS;
}

void
traverseDirectory(const std::string &searchPath, std::string &target, bool isRecursive,
                  bool ignoresCase) {
    int pid = getpid();

    DIR *dirp;
    struct dirent *direntp;


    if ((dirp = opendir(searchPath.c_str())) == nullptr) {
        perror("Failed to open directory");
//        return 1;
        return;
    }


    while ((direntp = readdir(dirp)) != nullptr) {
        std::string originalFilename = direntp->d_name;
        if (originalFilename.empty() || originalFilename == "." || originalFilename == "..") {
            continue;
        }

        std::string filename01;
        // not mutating originalFilename for output at the end
        handleCase(target, ignoresCase, originalFilename, filename01);

        std::string fullPath = searchPath;
        fullPath.append("/");
        fullPath.append(originalFilename);


        struct stat statbuf{};
        if (stat(fullPath.c_str(), &statbuf) == -1) {
            perror("Failed to get file status");
        } else {
            if (statbuf.st_mode & S_IFDIR) {
                if (isRecursive) {
                    traverseDirectory(fullPath, target, isRecursive, ignoresCase);
                }

                //it's a directory
            } else if (statbuf.st_mode & S_IFREG) {
                if (target == filename01) {
                    printf("%d: %s: %s\n", pid, originalFilename.c_str(), fullPath.c_str());
                } else {
                    // DO NOTHING -- Nothing found
                }
            } else {
                //something else
            }

        }
    }
    while ((closedir(dirp) == -1) && (errno == EINTR)) { ; }
}

void handleCase(std::string &target, bool ignoresCase, std::string &originalFilename,
                std::string &filename01) {
    filename01 = originalFilename;
    if (ignoresCase) {
        filename01 = toLower(originalFilename);
        target = toLower(target);
    }
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

    for (auto &target: targets) {
        std::cout << target << std::endl;
    }
    std::cout << "\n" << std::endl;

}
