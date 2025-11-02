/*
 * cp.c - Copy files and directories for Vest-OS
 *
 * This is a basic implementation of the cp utility that provides
 * file and directory copying functionality with common options.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>

#define CP_VERSION "1.0.0"
#define BUFFER_SIZE 4096

/* Global flags */
static int recursive = 0;
static int preserve = 0;
static int interactive = 0;
static int force = 0;
static int verbose = 0;
static int no_clobber = 0;

/* Function prototypes */
static void print_usage(const char *program);
static void print_version(void);
static void parse_arguments(int argc, char *argv[]);
static int copy_file(const char *src, const char *dst);
static int copy_directory(const char *src, const char *dst);
static int copy_file_data(const char *src, const char *dst);
static int copy_attributes(const char *src, const char *dst);
static int is_directory(const char *path);
static int file_exists(const char *path);
static int confirm_overwrite(const char *path);
static void show_copy_progress(const char *src, const char *dst);
static int create_directory(const char *path);

int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    /* Parse command line arguments */
    parse_arguments(argc, argv);

    /* Determine source and destination */
    int src_count = argc - 2;
    const char **sources = (const char **)&argv[1];
    const char *destination = argv[argc - 1];

    /* Check if destination is a directory when copying multiple sources */
    if (src_count > 1 && !is_directory(destination)) {
        fprintf(stderr, "cp: target '%s' is not a directory\n", destination);
        return 1;
    }

    /* Copy each source to destination */
    int failed = 0;
    for (int i = 0; i < src_count; i++) {
        char dst_path[1024];
        const char *src = sources[i];

        /* Build destination path */
        if (is_directory(destination)) {
            const char *basename = strrchr(src, '/');
            if (basename == NULL) {
                basename = src;
            } else {
                basename++;
            }
            snprintf(dst_path, sizeof(dst_path), "%s/%s", destination, basename);
        } else {
            strncpy(dst_path, destination, sizeof(dst_path) - 1);
            dst_path[sizeof(dst_path) - 1] = '\0';
        }

        /* Check if we should overwrite */
        if (file_exists(dst_path) && no_clobber) {
            fprintf(stderr, "cp: not overwriting '%s'\n", dst_path);
            failed = 1;
            continue;
        }

        if (file_exists(dst_path) && interactive && !confirm_overwrite(dst_path)) {
            continue;
        }

        /* Perform the copy */
        int result;
        if (is_directory(src)) {
            if (!recursive) {
                fprintf(stderr, "cp: -r not specified; omitting directory '%s'\n", src);
                failed = 1;
                continue;
            }
            result = copy_directory(src, dst_path);
        } else {
            result = copy_file(src, dst_path);
        }

        if (result != 0) {
            failed = 1;
        } else if (verbose) {
            show_copy_progress(src, dst_path);
        }
    }

    return failed ? 1 : 0;
}

static void print_usage(const char *program) {
    printf("Usage: %s [OPTION]... SOURCE... DESTINATION\n", program);
    printf("Copy SOURCE to DESTINATION, or multiple SOURCE(s) to DIRECTORY.\n\n");
    printf("Options:\n");
    printf("  -r, -R, --recursive   copy directories recursively\n");
    printf("  -p, --preserve        preserve file attributes (mode, ownership, timestamps)\n");
    printf("  -i, --interactive     prompt before overwrite\n");
    printf("  -f, --force           if an existing destination file cannot be opened,\n");
    printf("                        remove it and try again (this option is ignored when\n");
    printf("                        the -n option is also used)\n");
    printf("  -n, --no-clobber      do not overwrite an existing file\n");
    printf("  -v, --verbose         explain what is being done\n");
    printf("      --help            display this help and exit\n");
    printf("      --version         output version information and exit\n");
}

static void print_version(void) {
    printf("cp (Vest-OS coreutils) %s\n", CP_VERSION);
    printf("Copyright (C) 2024 Vest-OS Project\n");
    printf("License: MIT\n");
    printf("This is free software: you are free to change and redistribute it.\n");
}

static void parse_arguments(int argc, char *argv[]) {
    /* Parse options and remove them from argv */
    int i, j;
    for (i = 1; i < argc - 1; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "--help") == 0) {
                print_usage(argv[0]);
                exit(0);
            } else if (strcmp(argv[i], "--version") == 0) {
                print_version();
                exit(0);
            } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "-R") == 0 ||
                       strcmp(argv[i], "--recursive") == 0) {
                recursive = 1;
            } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--preserve") == 0) {
                preserve = 1;
            } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interactive") == 0) {
                interactive = 1;
            } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--force") == 0) {
                force = 1;
            } else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--no-clobber") == 0) {
                no_clobber = 1;
            } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
                verbose = 1;
            } else {
                fprintf(stderr, "cp: invalid option -- '%s'\n", argv[i]);
                fprintf(stderr, "Try 'cp --help' for more information.\n");
                exit(1);
            }

            /* Remove option from argv */
            for (j = i; j < argc - 1; j++) {
                argv[j] = argv[j + 1];
            }
            argc--;
            i--;
        }
    }
}

static int copy_file(const char *src, const char *dst) {
    struct stat src_stat, dst_stat;

    /* Get source file information */
    if (stat(src, &src_stat) != 0) {
        fprintf(stderr, "cp: cannot stat '%s': %s\n", src, strerror(errno));
        return -1;
    }

    /* Check if source and destination are the same file */
    if (stat(dst, &dst_stat) == 0) {
        if (src_stat.st_dev == dst_stat.st_dev && src_stat.st_ino == dst_stat.st_ino) {
            fprintf(stderr, "cp: '%s' and '%s' are the same file\n", src, dst);
            return -1;
        }
    }

    /* Copy file data */
    if (copy_file_data(src, dst) != 0) {
        return -1;
    }

    /* Copy attributes if requested */
    if (preserve) {
        copy_attributes(src, dst);
    }

    return 0;
}

static int copy_directory(const char *src, const char *dst) {
    DIR *dir;
    struct dirent *entry;
    int failed = 0;

    /* Create destination directory */
    if (create_directory(dst) != 0) {
        fprintf(stderr, "cp: cannot create directory '%s': %s\n", dst, strerror(errno));
        return -1;
    }

    /* Copy attributes if requested */
    if (preserve) {
        copy_attributes(src, dst);
    }

    /* Open source directory */
    dir = opendir(src);
    if (dir == NULL) {
        fprintf(stderr, "cp: cannot open directory '%s': %s\n", src, strerror(errno));
        return -1;
    }

    /* Copy directory contents */
    while ((entry = readdir(dir)) != NULL) {
        char src_path[1024], dst_path[1024];

        /* Skip . and .. */
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        /* Build full paths */
        snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);
        snprintf(dst_path, sizeof(dst_path), "%s/%s", dst, entry->d_name);

        /* Recursively copy */
        int result;
        if (is_directory(src_path)) {
            result = copy_directory(src_path, dst_path);
        } else {
            result = copy_file(src_path, dst_path);
        }

        if (result != 0) {
            failed = 1;
        } else if (verbose) {
            show_copy_progress(src_path, dst_path);
        }
    }

    closedir(dir);
    return failed ? -1 : 0;
}

static int copy_file_data(const char *src, const char *dst) {
    int src_fd, dst_fd;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read, bytes_written;

    /* Open source file */
    src_fd = open(src, O_RDONLY);
    if (src_fd == -1) {
        fprintf(stderr, "cp: cannot open '%s' for reading: %s\n", src, strerror(errno));
        return -1;
    }

    /* Open destination file */
    dst_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst_fd == -1) {
        fprintf(stderr, "cp: cannot open '%s' for writing: %s\n", dst, strerror(errno));
        close(src_fd);
        return -1;
    }

    /* Copy data */
    while ((bytes_read = read(src_fd, buffer, BUFFER_SIZE)) > 0) {
        bytes_written = write(dst_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            fprintf(stderr, "cp: write error: %s\n", strerror(errno));
            close(src_fd);
            close(dst_fd);
            return -1;
        }
    }

    if (bytes_read == -1) {
        fprintf(stderr, "cp: read error: %s\n", strerror(errno));
        close(src_fd);
        close(dst_fd);
        return -1;
    }

    close(src_fd);
    close(dst_fd);
    return 0;
}

static int copy_attributes(const char *src, const char *dst) {
    struct stat src_stat;

    if (stat(src, &src_stat) != 0) {
        return -1;
    }

    /* Copy permissions */
    if (chmod(dst, src_stat.st_mode) != 0) {
        fprintf(stderr, "cp: failed to preserve permissions for '%s': %s\n", dst, strerror(errno));
        return -1;
    }

    /* Copy ownership (if running as root) */
    if (getuid() == 0) {
        if (chown(dst, src_stat.st_uid, src_stat.st_gid) != 0) {
            fprintf(stderr, "cp: failed to preserve ownership for '%s': %s\n", dst, strerror(errno));
            return -1;
        }
    }

    /* Copy timestamps */
    struct timeval times[2];
    times[0].tv_sec = src_stat.st_atime;
    times[0].tv_usec = 0;
    times[1].tv_sec = src_stat.st_mtime;
    times[1].tv_usec = 0;

    if (utimes(dst, times) != 0) {
        fprintf(stderr, "cp: failed to preserve timestamps for '%s': %s\n", dst, strerror(errno));
        return -1;
    }

    return 0;
}

static int is_directory(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        return 0;
    }
    return S_ISDIR(statbuf.st_mode);
}

static int file_exists(const char *path) {
    return access(path, F_OK) == 0;
}

static int confirm_overwrite(const char *path) {
    char response[10];
    printf("cp: overwrite '%s'? ", path);
    fflush(stdout);

    if (fgets(response, sizeof(response), stdin) == NULL) {
        return 0;
    }

    return (response[0] == 'y' || response[0] == 'Y');
}

static void show_copy_progress(const char *src, const char *dst) {
    printf("'%s' -> '%s'\n", src, dst);
}

static int create_directory(const char *path) {
    if (mkdir(path, 0755) == 0) {
        return 0;
    }

    if (errno == EEXIST) {
        /* Directory already exists, that's fine */
        return 0;
    }

    return -1;
}