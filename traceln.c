
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>

void print_usage(char *name) {
    fprintf(stderr, "Usage: %s path\n", name);
}

void print_error() {
    fprintf(stderr, "%s\n", strerror(errno));
}

int main(int argc, char *const * argv) {
    // Given filename argument?
    if (argc != 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    char buf[PATH_MAX];
    char path[PATH_MAX];
    strcpy(path, argv[1]);
    bool found_one = false;
    ssize_t ret;
    while ((ret = readlink(path, buf, PATH_MAX)) != -1) {
        if (ret+1 >= PATH_MAX) {
            // (The +1 is for null terminator which we are going to add.)
            // This should be impossible.
            fprintf(stderr, "Path length longer than PATH_MAX.\n");
            return EXIT_FAILURE;
        }
        printf("%s\n", path);
        // Change dir to the where ever prev sym link is in case next one is
        // relative.
        if (chdir(dirname(path)) == -1) {
            print_error();
            return EXIT_FAILURE;
        }
        memcpy(path, buf, ret);
        path[ret] = '\0';
        found_one = true;
    }

    // EINVAL means we passed readlink a path which was not to a sym link,
    // which is ok as long as we traced at least one sym link.
    // Any other error is considered invalid, so let the user know.
    if (errno != EINVAL) {
        print_error();
        return EXIT_FAILURE;
    } else if (!found_one) {
        fprintf(stderr, "Not a symbolic link.\n");
        return EXIT_FAILURE;
    }

    // If all went well, the final path name (the first non-link) is in path and
    // needs to be printed.
    printf("%s\n", path);

    return EXIT_SUCCESS;
}

