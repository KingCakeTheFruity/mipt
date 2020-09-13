#include <assert.h>
#include <ctype.h>
#include<fcntl.h>
#include <io.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\stat.h>

const int MAXSTRLEN = 99;
const int MAXSTRS   = 10000;

enum SETTINGS{
    STROFA_SIZE = 14,
    RYTHM_DEPTH = 4
};

enum ERRORS {
    ERROR_FILE_NOT_FOUND = -10,
    ERROR_BIG_FILE,
    ERROR_MALLOC_FAIL
};

struct Line {
    unsigned char *string;
    int len;
    unsigned char ending[RYTHM_DEPTH + 1];
    int strofa_index;
};

typedef struct Line Line_t;

struct File {
    char *name;
    FILE *file_ptr;
    struct stat info;
    char *text;
    int lines_cnt;
    Line_t **lines;
};

typedef struct File File_t;

void qqh_sort(void *arr, int elem_cnt, size_t elem_size, int (*comp)(void *elem1, void *elem2));
int compare_lines(const void **elem1, const void **elem2);
int reverse_compare_lines(const void **elem1, const void **elem2);
int rythm_compare_lines(const void **elem1, const void **elem2);

void free_memory(Line_t **lines, char *file_text);
int read_file(File_t *file, char *name);
int read_lines(File_t *file);
void print_lines(char *file_names, Line_t **lines, int lines_cnt);

void calculate_eding(Line_t *line);
char rythming_lines(Line_t *first, Line_t *second, int depth);
void gen_strofa(Line_t **lines, int lines_cnt, unsigned int *buffer, int rythm_depth);

void print_error(int error);

int main(const int argc, const char **argv) {
    setlocale(LC_ALL,"Russian");


    char *fin_name  = "onegin.txt";
    char *fout_name = "oneginized.txt";
    if (argc > 1) {
        fin_name = argv[1];
    }
    if (argc > 2) {
        fout_name = argv[2];
    }

    File_t fin;
    int ret = read_file(&fin, fin_name);
    if (ret < 0) {
        print_error(ret);
        free_memory(fin.lines, fin.text);
    }
    if (fin.lines_cnt < 0) {
        print_error(fin.lines_cnt);
        free_memory(fin.lines, fin.text);
        return 0;
    }

    qsort(fin.lines, fin.lines_cnt, sizeof(Line_t*), compare_lines);
    printf("%d lines are read!\n", fin.lines_cnt);
    print_lines(fout_name, fin.lines, fin.lines_cnt);

    int buffer[STROFA_SIZE];
    gen_strofa(fin.lines, fin.lines_cnt, buffer, RYTHM_DEPTH);
    for (int i = 0; i < STROFA_SIZE; ++i) {
        printf("%s\n", fin.lines[buffer[i]]->string);
    }

    free_memory(fin.lines, fin.text);

    return 0;

}

char is_russian_letter(unsigned char c) {
    return (c >= (unsigned char) '�' && c <= (unsigned char) '�') || (c >= (unsigned char) '�' && c <= (unsigned char) '�');
}

char is_letter(unsigned char c) {
    return isalpha(c) || is_russian_letter(c);
}

void swap_ptrs(void **first, void **second) {
    void *tmp = *second;
    *second = *first;
    *first = tmp;
}

void qqh_sort(void *arr, int elem_cnt, size_t elem_size, int (*comp)(void *elem1, void *elem2)) {
    assert(arr);

    for (int i = 0; i < elem_cnt; ++i) {
        for (int j = 0; j < elem_cnt - 1; ++j) {
            void *first = arr + j * elem_size;
            void *second = arr + (j + 1) * elem_size;
            if (comp(first, second) > 0) {
                swap_ptrs(first, second);
            }
        }
    }
}

int compare_lines(const void **elem1, const void **elem2) {
    unsigned char* str1 = ((Line_t*)(*elem1))->string;
    unsigned char* str2 = ((Line_t*)(*elem2))->string;

    int i = 0;
    int j = 0;
    while (str1[i] && str2[j]) {
        while (!is_letter(str1[i]) && str1[i]) {
            ++i;
        }
        while (!is_letter(str2[j]) && str2[j]) {
            ++j;
        }

        if (str1[i] != str2[i] || str1[i] * str2[i] == 0) {
            return str1[i] - str2[j];
        }

        ++i;
        ++j;
    }
    return str1[i] - str2[j];
}

int reverse_compare_lines(const void **elem1, const void **elem2) {
    return -compare_lines(elem1, elem2);
}

int rythm_compare_lines(const void **elem1, const void **elem2) {
    Line_t *line1 = *elem1;
    Line_t *line2 = *elem2;
    unsigned char* str1 = line1->string;
    unsigned char* str2 = line2->string;

    int i = line1->len - 1;
    int j = strlen(line2->string) - 1;
    while (i >= 0 && j >= 0) {
        while (!is_letter(str1[i]) && str1[i]) {
            --i;
        }
        while (!is_letter(str2[j]) && str2[j]) {
            --j;
        }

        if (str1[i] != str2[i] || i * j == 0) {
            return str1[i] - str2[j];
        }

        --i;
        --j;
    }
    return str1[i] - str2[j];
}

void free_memory(Line_t **lines, char *file_text) {
    assert(lines);
    assert(file_text);

    Line_t **lines_ptr = lines;
    for (int i = 0; i < MAXSTRS; ++i) {
        free(*lines_ptr);
        ++lines_ptr;
    }
    free(lines);
    free(file_text);
}

int read_file(File_t *file, char *name) {
    assert(file);
    assert(name);

    file->name = name;
    stat(name, &(file->info));

    //file->lines = calloc(MAXSTRS, sizeof(Line_t*));
    file->text = calloc(file->info.st_size + 1, sizeof(char));
    if (!file->text) {
        return ERROR_MALLOC_FAIL;
    }
    return file->lines_cnt = read_lines(file);
}

int read_lines(File_t *file) {
    assert(file);

    printf("%s\n", file->name);
    file->file_ptr = open(file->name, O_BINARY);
    if (!file->file_ptr) {
        return ERROR_FILE_NOT_FOUND;
    }

    read(file->file_ptr, file->text, file->info.st_size);
    file->lines = calloc(MAXSTRS, sizeof(Line_t*));
    if (!file->lines) {
        return ERROR_MALLOC_FAIL;
    }

    char *c = file->text;
    int lines_cnt = -1;
    int line_len = 0;
    while (*c) {
        ++lines_cnt;
        if (lines_cnt == MAXSTRS - 1) {
            return ERROR_BIG_FILE;
        }

        file->lines[lines_cnt] = calloc(1, sizeof(Line_t));
        if (!file->lines[lines_cnt]) {
            return ERROR_MALLOC_FAIL;
        }
        Line_t *line_ptr = file->lines[lines_cnt];
        line_ptr->string = c;

        while(*c != '\n') {
            ++line_len;
            ++c;
        }
        *c = '\0';
        ++c;

        line_ptr->len = strlen(line_ptr->string);
        line_len = 0;
        calculate_ending(line_ptr);
        line_ptr->strofa_index = lines_cnt % STROFA_SIZE;
    }
    file->lines_cnt = lines_cnt;

    return lines_cnt + 1;
}

void print_lines(char *file_name, Line_t **lines, int lines_cnt) {
    FILE *fout = fopen(file_name, "w");
    for (int i = 0; i < lines_cnt; ++i) {
        fprintf(fout, "%s", lines[i]->string);
    }

    fclose(fout);
}

void calculate_ending(Line_t *line) {
    int i = line->len - 1;
    while(!is_letter(line->string[i]) && i) {
        --i;
    }
    if (!i) {
        return;
    }

    for (int j = 0; j < RYTHM_DEPTH; ++j) {
        line->ending[j] = line->string[i - RYTHM_DEPTH + j + 1];
    }
}

char rythming_lines(Line_t *first, Line_t *second, int depth) {
    if (first == second) {
        return 0;
    }

    char *str1 = first->string;
    char *str2 = second->string;

    if (strcmp(first->ending + 1, second->ending + 1) || first->ending[0] == second->ending[0]) {
        return 0;
    }
    return 1;
}

void gen_strofa(Line_t **lines, int lines_cnt, unsigned int *buffer, int rythm_depth) {
    assert(lines);
    assert(buffer);

    srand(time(NULL));
    for (int i = 0; i < STROFA_SIZE; ++i) {
        int itter = 0;
        while (1) {
            ++itter;
            if (itter == 100) {
                i = 0;
                continue;
            }

            int line_index = (((int) rand()) % (lines_cnt / 14)) * 14 + i;
            Line_t *line = lines[line_index];
            if (i == 0 || i == 1 || i == 4 || i == 6 || i == 8 || i == 9 || i == 12) {  // COMMENTS
                buffer[i] = line_index;
                break;
            } else if (i == 5 || i == 7 || i == 10 || i == 13) {
                if (rythming_lines(line, lines[buffer[i - 1]], rythm_depth)) {
                    buffer[i] = line_index;
                    break;
                }
            } else if (i == 2 || i == 3) {
                if (rythming_lines(line, lines[buffer[i - 2]], rythm_depth)) {
                    buffer[i] = line_index;
                    break;
                }
            } else {
                if (rythming_lines(line, lines[buffer[i - 3]], rythm_depth)) {
                    buffer[i] = line_index;
                    break;
                }
            }
        }
    }
}

void print_error(int error) {
    if (error == ERROR_FILE_NOT_FOUND) {
        printf("[ERR] File not found!\n");
    } else if (error == ERROR_BIG_FILE) {
        printf("[ERR] Can't handle such a big file!\n");
    } else if (error == ERROR_MALLOC_FAIL) {
        printf("[ERR] Can't allocate memory\n");
    } else {
        printf("[ERR](~!~)WERROREHUTGEERRORF(~!~)[ERR]\n");
    }
}

