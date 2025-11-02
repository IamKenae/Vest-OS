/*
 * ls.c - List directory contents for Vest-OS
 *
 * This is a basic implementation of the ls utility that provides
 * directory listing functionality with common options.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <errno.h>

#define LS_VERSION "1.0.0"

/* Structure to hold file information */
typedef struct {
    char name[256];
    char path[512];
    struct stat statbuf;
    int is_dir;
    int is_link;
    int is_exec;
} file_info_t;

/* Global flags */
static int show_all = 0;
static int long_format = 0;
static int show_hidden = 0;
static int reverse_order = 0;
static int size_sort = 0;
static int time_sort = 0;
static int color_output = 1;
static int recursive = 0;

/* Color codes */
#define COLOR_RESET   "\033[0m"
#define COLOR_DIR     "\033[34m"   /* Blue */
#define COLOR_EXEC    "\033[32m"   /* Green */
#define COLOR_LINK    "\033[36m"   /* Cyan */
#define COLOR_HIDDEN  "\033[37m"   /* Gray */

/* Function prototypes */
static void print_usage(const char *program);
static void print_version(void);
static void parse_arguments(int argc, char *argv[], char **dirs, int *dir_count);
static int collect_files(const char *path, file_info_t **files, int *count);
static void sort_files(file_info_t *files, int count);
static int compare_name(const void *a, const void *b);
static int compare_size(const void *a, const void *b);
static int compare_time(const void *a, const void *b);
static void print_files(file_info_t *files, int count, const char *path);
static void print_long_format(const file_info_t *file);
static void print_simple_format(const file_info_t *file);
static const char *get_color_for_file(const file_info_t *file);
static void print_file_info(const file_info_t *file, const char *base_path);
static int is_executable(const struct stat *statbuf);

int main(int argc, char *argv[]) {
    char *dirs[32] = {NULL};
    int dir_count = 0;

    /* Parse command line arguments */
    parse_arguments(argc, argv, dirs, &dir_count);

    /* If no directories specified, use current directory */
    if (dir_count == 0) {
        dirs[0] = ".";
        dir_count = 1;
    }

    /* Process each directory */
    for (int i = 0; i < dir_count; i++) {
        file_info_t *files = NULL;
        int file_count = 0;

        /* Collect files from directory */
        if (collect_files(dirs[i], &files, &file_count) != 0) {
            fprintf(stderr, "ls: cannot access '%s': %s\n", dirs[i], strerror(errno));
            continue;
        }

        /* Sort files */
        sort_files(files, file_count);

        /* Print directory header if multiple directories */
        if (dir_count > 1) {
            printf("%s:\n", dirs[i]);
        }

        /* Print files */
        print_files(files, file_count, dirs[i]);

        /* Add newline between directories */
        if (i < dir_count - 1) {
            printf("\n");
        }

        /* Clean up */
        free(files);
    }

    return 0;
}

static void print_usage(const char *program) {
    printf("Usage: %s [OPTION]... [FILE]...\n", program);
    printf("List information about the FILEs (the current directory by default).\n\n");
    printf("Options:\n");
    printf("  -a, --all              do not ignore entries starting with .\n");
    printf("  -A, --almost-all      do not list implied . and ..\n");
    printf("  -l                     use a long listing format\n");
    printf("  -r, --reverse         reverse order while sorting\n");
    printf("  -S                     sort by file size, largest first\n");
    printf("  -t                     sort by modification time, newest first\n");
    printf("  -R, --recursive       list subdirectories recursively\n");
    printf("      --color[=WHEN]    colorize the output (auto, always, never)\n");
    printf("      --help            display this help and exit\n");
    printf("      --version         output version information and exit\n");
}

static void print_version(void) {
    printf("ls (Vest-OS coreutils) %s\n", LS_VERSION);
    printf("Copyright (C) 2024 Vest-OS Project\n");
    printf("License: MIT\n");
    printf("This is free software: you are free to change and redistribute it.\n");
}

static void parse_arguments(int argc, char *argv[], char **dirs, int *dir_count) {
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            /* Handle options */
            if (strcmp(argv[i], "--help") == 0) {
                print_usage(argv[0]);
                exit(0);
            } else if (strcmp(argv[i], "--version") == 0) {
                print_version();
                exit(0);
            } else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0) {
                show_all = 1;
                show_hidden = 1;
            } else if (strcmp(argv[i], "-A") == 0 || strcmp(argv[i], "--almost-all") == 0) {
                show_all = 1;
            } else if (strcmp(argv[i], "-l") == 0) {
                long_format = 1;
            } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--reverse") == 0) {
                reverse_order = 1;
            } else if (strcmp(argv[i], "-S") == 0) {
                size_sort = 1;
            } else if (strcmp(argv[i], "-t") == 0) {
                time_sort = 1;
            } else if (strcmp(argv[i], "-R") == 0 || strcmp(argv[i], "--recursive") == 0) {
                recursive = 1;
            } else if (strncmp(argv[i], "--color", 7) == 0) {
                color_output = 1;
            } else {
                fprintf(stderr, "ls: invalid option -- '%s'\n", argv[i]);
                fprintf(stderr, "Try 'ls --help' for more information.\n");
                exit(1);
            }
        } else {
            /* It's a directory name */
            if (*dir_count < 32) {
                dirs[*dir_count] = argv[i];
                (*dir_count)++;
            }
        }
    }
}

static int collect_files(const char *path, file_info_t **files, int *count) {
    DIR *dir;
    struct dirent *entry;
    file_info_t *file_list = NULL;
    int file_count = 0;
    int capacity = 0;

    dir = opendir(path);
    if (dir == NULL) {
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        /* Skip hidden files unless requested */
        if (!show_hidden && entry->d_name[0] == '.') {
            continue;
        }

        /* Skip . and .. unless -a is specified */
        if (!show_all && (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)) {
            continue;
        }

        /* Expand array if needed */
        if (file_count >= capacity) {
            capacity = capacity == 0 ? 16 : capacity * 2;
            file_list = realloc(file_list, capacity * sizeof(file_info_t));
            if (file_list == NULL) {
                closedir(dir);
                return -1;
            }
        }

        /* Fill file info */
        file_info_t *info = &file_list[file_count];
        strncpy(info->name, entry->d_name, sizeof(info->name) - 1);
        info->name[sizeof(info->name) - 1] = '\0';

        snprintf(info->path, sizeof(info->path), "%s/%s", path, entry->d_name);

        if (stat(info->path, &info->statbuf) != 0) {
            /* If stat fails, use lstat for broken symlinks */
            if (lstat(info->path, &info->statbuf) != 0) {
                continue; /* Skip files we can't stat */
            }
        }

        info->is_dir = S_ISDIR(info->statbuf.st_mode);
        info->is_link = S_ISLNK(info->statbuf.st_mode);
        info->is_exec = is_executable(&info->statbuf);

        file_count++;
    }

    closedir(dir);

    *files = file_list;
    *count = file_count;
    return 0;
}

static void sort_files(file_info_t *files, int count) {
    if (size_sort) {
        qsort(files, count, sizeof(file_info_t), compare_size);
    } else if (time_sort) {
        qsort(files, count, sizeof(file_info_t), compare_time);
    } else {
        qsort(files, count, sizeof(file_info_t), compare_name);
    }

    if (reverse_order) {
        /* Reverse the array */
        for (int i = 0; i < count / 2; i++) {
            file_info_t temp = files[i];
            files[i] = files[count - 1 - i];
            files[count - 1 - i] = temp;
        }
    }
}

static int compare_name(const void *a, const void *b) {
    const file_info_t *fa = (const file_info_t *)a;
    const file_info_t *fb = (const file_info_t *)b;
    return strcasecmp(fa->name, fb->name);
}

static int compare_size(const void *a, const void *b) {
    const file_info_t *fa = (const file_info_t *)a;
    const file_info_t *fb = (const file_info_t *)b;

    if (fa->statbuf.st_size < fb->statbuf.st_size) return 1;
    if (fa->statbuf.st_size > fb->statbuf.st_size) return -1;
    return compare_name(a, b);
}

static int compare_time(const void *a, const void *b) {
    const file_info_t *fa = (const file_info_t *)a;
    const file_info_t *fb = (const file_info_t *)b;

    if (fa->statbuf.st_mtime < fb->statbuf.st_mtime) return 1;
    if (fa->statbuf.st_mtime > fb->statbuf.st_mtime) return -1;
    return compare_name(a, b);
}

static void print_files(file_info_t *files, int count, const char *path) {
    for (int i = 0; i < count; i++) {
        if (long_format) {
            print_long_format(&files[i]);
        } else {
            print_simple_format(&files[i]);
        }

        /* Handle recursive listing */
        if (recursive && files[i].is_dir && strcmp(files[i].name, ".") != 0 &&
            strcmp(files[i].name, "..") != 0) {
            printf("\n");
            file_info_t *subfiles = NULL;
            int subcount = 0;
            char subpath[512];

            snprintf(subpath, sizeof(subpath), "%s/%s", path, files[i].name);

            if (collect_files(subpath, &subfiles, &subcount) == 0) {
                sort_files(subfiles, subcount);
                printf("%s:\n", subpath);
                print_files(subfiles, subcount, subpath);
                free(subfiles);
            }
        }
    }

    if (!long_format && count > 0) {
        printf("\n");
    }
}

static void print_long_format(const file_info_t *file) {
    char permissions[11];
    struct passwd *pwd;
    struct group *grp;
    char time_str[20];

    /* File type and permissions */
    if (file->is_dir) {
        permissions[0] = 'd';
    } else if (file->is_link) {
        permissions[0] = 'l';
    } else {
        permissions[0] = '-';
    }

    permissions[1] = (file->statbuf.st_mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (file->statbuf.st_mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (file->statbuf.st_mode & S_IXUSR) ? 'x' : '-';
    permissions[4] = (file->statbuf.st_mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (file->statbuf.st_mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (file->statbuf.st_mode & S_IXGRP) ? 'x' : '-';
    permissions[7] = (file->statbuf.st_mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (file->statbuf.st_mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (file->statbuf.st_mode & S_IXOTH) ? 'x' : '-';
    permissions[10] = '\0';

    /* Get user and group names */
    pwd = getpwuid(file->statbuf.st_uid);
    grp = getgrgid(file->statbuf.st_gid);

    /* Format time */
    struct tm *tm_info = localtime(&file->statbuf.st_mtime);
    strftime(time_str, sizeof(time_str), "%b %d %H:%M", tm_info);

    /* Print the long format line */
    printf("%s %2ld ", permissions, file->statbuf.st_nlink);

    if (pwd) {
        printf("%-8s ", pwd->pw_name);
    } else {
        printf("%-8ld ", file->statbuf.st_uid);
    }

    if (grp) {
        printf("%-8s ", grp->gr_name);
    } else {
        printf("%-8ld ", file->statbuf.st_gid);
    }

    printf("%8ld ", file->statbuf.st_size);
    printf("%s ", time_str);

    if (color_output) {
        printf("%s%s%s", get_color_for_file(file), file->name, COLOR_RESET);
    } else {
        printf("%s", file->name);
    }

    /* Handle symbolic links */
    if (file->is_link) {
        char link_target[256];
        ssize_t len = readlink(file->path, link_target, sizeof(link_target) - 1);
        if (len != -1) {
            link_target[len] = '\0';
            printf(" -> %s", link_target);
        }
    }

    printf("\n");
}

static void print_simple_format(const file_info_t *file) {
    if (color_output) {
        printf("%s%s%s  ", get_color_for_file(file), file->name, COLOR_RESET);
    } else {
        printf("%s  ", file->name);
    }
}

static const char *get_color_for_file(const file_info_t *file) {
    if (file->is_link) {
        return COLOR_LINK;
    } else if (file->is_dir) {
        return COLOR_DIR;
    } else if (file->is_exec) {
        return COLOR_EXEC;
    } else if (file->name[0] == '.') {
        return COLOR_HIDDEN;
    }
    return "";
}

static int is_executable(const struct stat *statbuf) {
    return (statbuf->st_mode & S_IXUSR) ||
           (statbuf->st_mode & S_IXGRP) ||
           (statbuf->st_mode & S_IXOTH);
}