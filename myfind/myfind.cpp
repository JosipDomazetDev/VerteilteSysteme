/**
 *
 * myfind.cpp
 * Verteilte Systeme
 */

#include <getopt.h>
#include <vector>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cassert>
#include <algorithm>

/* globale Variable fuer den Programmnamen */
char *program_name = nullptr;

void extractArguments(int argc, char *argv[], std::string &searchPath, std::vector<std::string> &targets, int c,
                      bool &isRecursive, bool &ignoresCase, int error);

std::string toLower(std::string &filename);

void
handleCase(std::string &targets, bool ignoresCase, std::string &originalFilename, std::string &sanitizedFilename);

int
traverseDirectory(const std::string &searchPath, std::string &targets, bool isRecursive, bool ignoresCase);

/* Funktion print_usage() zur Ausgabe der usage Meldung */
void print_usage() {
    fprintf(stderr, "Usage: %s [-R] [-i] searchpath filename1 [filename2] ...[filenameN] \n", program_name);
    exit(EXIT_FAILURE);
}

std::string toLower(std::string &filename) {
    std::string copiedFilename = filename;
    std::transform(copiedFilename.begin(), copiedFilename.end(), copiedFilename.begin(),
                   [](unsigned char c) { return tolower(c); });

    return copiedFilename;
}

// Handle case insensitive -i option by simply converting both the target and the filename to lower case
// Does not mutate the originalFilename to keep the output at the end unaffected
void handleCase(std::string &target, bool ignoresCase, std::string &originalFilename,
                std::string &sanitizedFilename) {
    sanitizedFilename = originalFilename;
    if (ignoresCase) {
        sanitizedFilename = toLower(originalFilename);
        target = toLower(target);
    }
}

void extractArguments(int argc, char *argv[], std::string &searchPath, std::vector<std::string> &targets, int c,
                      bool &isRecursive, bool &ignoresCase, int error) {
    while ((c = getopt(argc, argv, "iR")) != EOF) {
        switch (c) {
            case 'i':
                ignoresCase = true;
                break;
            case 'R':
                isRecursive = true;
                break;
            case '?':
                /* unguelgtiges Argument */
                error = 1;
                break;
            default:
                /* unmoegliech */
                assert(0);
        }
    }

    if (error) {
        /* Optionen fehlerhaft ? */
        print_usage();
    }

    if ((argc < optind + 2)) {
        /* falsche Anzahl an Optionen */
        print_usage();
    }

    while (optind < argc) {
        /* aktuelles Argument: argv[optind] */

        if (searchPath.empty()) {
            // The first positional argument is the searchPath
            searchPath = argv[optind];
        } else {
            // Once the searchPath has been set, the remaining arguments are targets, add them to a vector
            targets.emplace_back(argv[optind]);
        }

        optind++;
    }


    /*
     * I used this for debugging purposes
     * std::cout << "=============== Used arguments ===============" << std::endl;
       std::cout << "isRecursive: " + std::to_string(isRecursive) << std::endl;
       std::cout << "ignoresCase: " + std::to_string(ignoresCase) << std::endl;
       std::cout << "searchPath: " + searchPath << std::endl;
       std::cout << "\nTargets: " << std::endl;

        for (auto &target: targets) {
        std::cout << target << std::endl;
        }
        std::cout << "\n" << std::endl;
     */

}

int
traverseDirectory(const std::string &searchPath, std::string &target, bool isRecursive,
                  bool ignoresCase) {
    int pid = getpid();

    DIR *dirp;
    struct dirent *direntp;

    // Open the contents of the directory specified in searchPath
    if ((dirp = opendir(searchPath.c_str())) == nullptr) {
        std::string msg = "Failed to open directory ";
        msg.append(searchPath);

        perror(msg.c_str());
        return EXIT_FAILURE;
    }

    while ((direntp = readdir(dirp)) != nullptr) {
        // Get the current filename
        std::string originalFilename = direntp->d_name;

        if (originalFilename.empty() || originalFilename == "." || originalFilename == "..") {
            // Those are not actual files, ignore them
            continue;
        }

        std::string sanitizedFilename;
        // Handle the -i option
        handleCase(target, ignoresCase, originalFilename, sanitizedFilename);

        // Keep track of the full path for the recursive search
        std::string fullPath = searchPath;
        fullPath.append("/");
        fullPath.append(originalFilename);


        struct stat statbuf{};
        if (stat(fullPath.c_str(), &statbuf) == -1) {
            return EXIT_FAILURE;
        } else {
            if (statbuf.st_mode & S_IFDIR) {
                // It's a directory, traverse it recursively if user used -R
                if (isRecursive) {
                    int result = traverseDirectory(fullPath, target, isRecursive, ignoresCase);

                    if (result == EXIT_FAILURE) {
                        return EXIT_FAILURE;
                    }
                }
            } else if (statbuf.st_mode & S_IFREG) {
                // It's a file, check if filename matches the target
                if (target == sanitizedFilename) {
                    // We have a match -- print it
                    printf("%d: %s: %s\n", pid, originalFilename.c_str(), fullPath.c_str());
                }
            }

        }
    }
    while ((closedir(dirp) == -1) && (errno == EINTR)) { ; }


    return EXIT_SUCCESS;
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

    // Read command lines arguments
    extractArguments(argc, argv, searchPath, targets, c, isRecursive, ignoresCase, error);

    pid_t pid, wpid;
    int status;

    // Create a fork for each target. Each child process will call traverseDirectory to search for a match.
    for (auto &target: targets) {

        pid = fork();

        if (pid == 0) {
            // Child process returns result of traverseDirectory, therefore the code below will not be executed by the children
            return traverseDirectory(searchPath, target, isRecursive, ignoresCase);

        } else if (pid == -1) {
            perror("Forking failed\n");
            return EXIT_FAILURE;
        }
    }


    // Wait for all children
    while ((wpid = wait(&status)) > 0) {

        /* check exit code of child after finishing */
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            // traverseDirectory failed and returned error code
            fprintf(stderr, "%s\n", "Error while waiting for child process");
        }

        if (wpid != -1)
            continue; /* different child process has terminated, continue waiting */

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

