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

/* globale Variable fuer den Programmnamen */
char *program_name = nullptr;

void extractArguments(int argc, char *argv[], std::string &searchPath, std::vector<std::string> &targets, int c,
                      bool &isRecursive, bool &ignoresCase, int error);

std::string toLower(std::string &filename);

void
handleCase(std::string &targets, bool ignoresCase, std::string &originalFilename, std::string &filename01);

int
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

    // Liest die Kommandozeilen-Parameter aus
    extractArguments(argc, argv, searchPath, targets, c, isRecursive, ignoresCase, error);

    pid_t pid, wpid;
    int status;
    // Für jedes target wird nun ein fork erstellt. Der Kindproess sucht mit der Methode traverseDirectory nach Matches.
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
            perror("Error while waiting for child process\n");
        }

        if (wpid != -1)
            continue; /* different child process has terminated, continue waiting */



        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}

int
traverseDirectory(const std::string &searchPath, std::string &target, bool isRecursive,
                  bool ignoresCase) {
    int pid = getpid();

    DIR *dirp;
    struct dirent *direntp;


    if ((dirp = opendir(searchPath.c_str())) == nullptr) {
        perror("Failed to open directory");
        return EXIT_FAILURE;
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
            return EXIT_FAILURE;
        } else {
            if (statbuf.st_mode & S_IFDIR) {
                // It's a directory, traverse it recursively if user used -R
                if (isRecursive) {
                    traverseDirectory(fullPath, target, isRecursive, ignoresCase);
                }
            } else if (statbuf.st_mode & S_IFREG) {
                // It's a file, check if filename matches the target
                if (target == filename01) {
                    printf("%d: %s: %s\n", pid, originalFilename.c_str(), fullPath.c_str());
                }
            }

        }
    }
    while ((closedir(dirp) == -1) && (errno == EINTR)) { ; }


    return 0;
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
