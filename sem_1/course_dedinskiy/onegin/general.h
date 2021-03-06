/**
    \file
    \brief General functions to be used in all projects
*/


/*
NOTES:
    +1) ����������, ��� ����� get_next_letter (docs), is_countable �� ��������� ��������
    +2) ���������� �������, ����������� �������� � ����� �� ��
    -) �������� ��������� ������ �� ��� ����
    ?4) ���� ������, �����������, ��� ���������� - ������������� ������, ���� ��������
    +5) assert � �� �����, � ��� �� �������������??
*/

#ifndef KCTF_GENERAL_H
#define KCTF_GENERAL_H

#include <assert.h>

//<KCTF> Everyday_staff =======================================================

const int KCTF_DEBUG_LEVEL = 2; ///< Just a mode for debugging

int           DEBUG_NUMBER = 1;   ///< Just an int for debugging
unsigned char DEBUG_LETTER = 'a'; ///< Just a char for debugging

//Just Joking

#define DEBUG_NUMBER_PRINT() printf("[deb] %d [deb]\n", DEBUG_NUMBER++);
#define DEBUG_LETTER_PRINT() printf("[deb] %c [deb]\n", DEBUG_LETTER++);

#define DEBUG_NUMBER_INCR() DEBUG_NUMBER++;
#define DEBUG_LETTER_INCR() DEBUG_LETTER++;

#define DEBUG(LEVEL) if (LEVEL <= KCTF_DEBUG_LEVEL)

const int INT_P = 7777777; /// Poison int

///  Return codes
enum RETURN_CODES {
    ERROR_FILE_NOT_FOUND = -10,
    ERROR_BIG_FILE,
    ERROR_MALLOC_FAIL,
    ERROR_NULL_OBJECT,
    ERROR_NO_RET_CODE,
    ERROR_BAD_ARGS,
    NULL_OBJ_OK = 0,
    RET_OK = 0,
};

//=============================================================================

/// Current project's setting
enum CURRENT_PROJECT_SETTINGS { // special for Onegin
    STROFA_SIZE = 14,
    RHYME_DEPTH = 4
};

/// Handmade stringview
struct Line {
    unsigned char *string;
    size_t len;
    int index; // for debug !#!@#@!#@!#@!#
    unsigned char ending[RHYME_DEPTH + 1]; // special for Onegin
};

/// Typedef for Line
typedef struct Line Line_t;

/// Struct to store file's information into
struct File {
    const char *name;
    size_t file_dscr;
    struct stat info;
    unsigned char *text;
    size_t lines_cnt;
    Line_t **lines;
};

/// Typedef for File
typedef struct File File_t;

/**
    \brief Quadratic sort

    Sorts array on-place with given comparator

    \param[in] arr array which needs to be sorted
    \param[in] elem_cnt count of elements, [0, elem_cnt) will be sorted
    \param[in] elem_size size of each element in bytes
    \param[in] comp comparator returning an int <0 of elem1<elem2, 0 if elem1=elem2, >0 if elem1>elem2
*/
void qqh_sort(void *arr, const size_t elem_cnt, const size_t elem_size, int (*comp)(const void *elem1, const void *elem2));

/**
    \brief Comparator for two lines

    Ignores everything that is not a Russian or English letter

    \param[in] elem1,elem2 elements to compare
    \return an int <0 if elem1<elem2, 0 if elem1=elem2, >0 if elem1>elem2
*/
int compare_lines_letters(const void *elem1, const void *elem2);

/**
    \brief Reversed comparator for two lines

    Ignores everything that is not a Russian or English letter

    \param[in] elem1,elem2 elements to compare
    \return an int >0 if elem1<elem2, 0 if elem1=elem2, 0 if elem1>elem2
*/
int reverse_compare_lines_letters(const void **elem1, const void **elem2);

/**
    \brief Calls all necessary free's

    Kind of destructor for the File structure

    \param[in] file object to be destroyed
*/
void free_memory_file(const File_t *file);

/**
    \brief Reads file

    Estimates file's size and allocates just enough memory + 1 byte for \0, then calls read_lines to fill buffer 'text' and index 'lines'

    \param[in] file object to be read to
    \param[in] name - filename to be read from
    \return 0 if file is read successfully, else error code <0
*/
int read_file(File_t *file, const char *name);

/**
    \brief Reads lines from file

    Stores them into given File_t object

    \param[in] file an object to write into, contains input file name
    \return 0 if file is read successfully, else error code <0
*/
int read_lines(File_t *file);

/**
    \brief Prints file into given file



    \param[in] file file containing text to write
    \param[in] output file name
*/
void print_file(const File_t *file, const char *fout_name, const char *mode);


/**
    \brief Checks if c is a Russian or an English letter

    .

    \param[in] c char to check
    \return true if c is a Russian or an English letter, else false
*/
int is_countable(const unsigned char c);

/**
    \brief Swaps contains of two ptrs

    .

    \param[in] first,second prts to swap
    \return
*/
void swap_ptrs(void **first, void **second);

/**
    \brief Prints text error message to standard output

    .

    \param[in] error code of error to print
*/
void print_error(int error);


//=============================================================================
/// @name IMPLEMENTATION
//=============================================================================
/// @{

int is_countable(const unsigned char c) {
    return isalnum(c);
}

void swap_ptrs(void **first, void **second) {
    assert(first);
    assert(second);

    void *tmp = *second;
    *second = *first;
    *first = tmp;
}

void qqh_sort(void *arr, const size_t elem_cnt, const size_t elem_size,
              int (*comp)(const void *elem1, const void *elem2)) {

    assert(arr);
    //assert(comp) ???

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

void get_next_letter(unsigned char **c) {
    unsigned char *cur_c = *c;
    while(!is_countable(*cur_c) && *cur_c) {
        ++cur_c;
    }
    *c = cur_c;
}

int compare_lines_letters(const void *elem1, const void *elem2) {
    unsigned char *first_c  = ((**(Line_t**)elem1).string);
    unsigned char *second_c = ((**(Line_t**)elem2).string);

    while (*first_c && *second_c) {
        get_next_letter(&first_c);
        get_next_letter(&second_c);

        if (*first_c != *second_c || (*first_c) * (*second_c) == 0) {
            return (int) *first_c - (int) *second_c;
        }

        ++first_c;
        ++second_c;
    }

    get_next_letter(&first_c);
    get_next_letter(&second_c);
    return (int) *first_c - (int) *second_c;
}

int reverse_compare_lines_letters(const void **elem1, const void **elem2) {
    return -compare_lines_letters(elem1, elem2);
}

void free_memory_file(const File_t *file) {
    assert(file);

    Line_t **lines_ptr = file->lines;
    for (int i = 0; i < file->lines_cnt; ++i) {
        free(*lines_ptr);
        ++lines_ptr;
    }
    free(file->lines);
    free(file->text);
}

int read_file(File_t *file, const char *name) {
    assert(file);
    assert(name);

    file->name = name;
    stat(name, &(file->info));

    file->text = calloc(file->info.st_size + 1, sizeof(char));
    if (!file->text) {
        return ERROR_MALLOC_FAIL;
    }
    return file->lines_cnt = read_lines(file);
}

int read_lines(File_t *file) {
    assert(file);

    DEBUG(1) {printf("Working with [%s] file\n", file->name);}

    file->file_dscr = open(file->name, O_BINARY);

    read(file->file_dscr, file->text, file->info.st_size);
    int lines_cnt = 0;
    for (unsigned char *c = file->text; *c; ++c) {
        lines_cnt += *c == '\n';
    }

    file->lines = calloc(lines_cnt, sizeof(Line_t*));
    if (file->lines == NULL) {
        return ERROR_MALLOC_FAIL;
    }

    unsigned char *c = file->text;
    lines_cnt = -1;
    int line_len = 0;
    while (*c) {
        ++lines_cnt;

        file->lines[lines_cnt] = calloc(1, sizeof(Line_t));
        Line_t *line_ptr = file->lines[lines_cnt];
        if (line_ptr == NULL) {
            return ERROR_MALLOC_FAIL;
        }
        line_ptr->string = c;
        line_ptr->index = lines_cnt;

        while(*c != '\n') {
            ++line_len;
            ++c;
            if (*c == '\r') {
                *c = '\0';
            }
        }
        *c = '\0';
        ++c;

        line_ptr->len = line_len;
        line_len = 0;
    }
    file->lines_cnt = lines_cnt + 1;

    return lines_cnt + 1;
}

void print_file(const File_t *file, const char *fout_name, const char *mode) {
    assert(file);
    assert(fout_name);

    FILE *fout = fopen(fout_name, mode);
    for (int i = 0; i < file->lines_cnt; ++i) {
        fprintf(fout, "%s\n", file->lines[i]->string);
    }

    fclose(fout);
}

void print_error(int error) {
    if (error == 0) {
        return;
    }

    if (error == ERROR_FILE_NOT_FOUND) {
        printf("[ERR] File not found!\n");
    } else if (error == ERROR_BIG_FILE) {
        printf("[ERR] Can't handle such a big file!\n");
    } else if (error == ERROR_MALLOC_FAIL) {
        printf("[ERR] Can't allocate memory\n");
    } else {
        printf("[ERR](~!~)WERRORHUTGEERRORF(~!~)[ERR]\n");
    }
}


// UNIT TESTS

int utest_compare_lines_letters() {
    srand(time(NULL));
    File_t file = {};
    read_file(&file, "utest_compare_lines_letters.txt");
    for (size_t itter = 0; itter < 1000; ++itter) {
        for (size_t i = 0; i < file.lines_cnt / 2; ++i) {
            const size_t x = rand() % file.lines_cnt;
            const size_t y = rand() % file.lines_cnt;
            Line_t *tmp = file.lines[x];
            file.lines[x] = file.lines[y];
            file.lines[y] = tmp;
        }

        qsort(file.lines, file.lines_cnt, sizeof(Line_t*), compare_lines_letters);

        for (int i = 0; i < file.lines_cnt - 1; ++i) {
            if (file.lines[i]->index > file.lines[i + 1]->index && compare_lines_letters(&file.lines[i], &file.lines[i + 1])) {
                printf("[ERR] \"%s\" > \"%s\"\n", file.lines[i]->string, file.lines[i + 1]->string);
                printf("[ ! ] indexes: %d > %d\n", file.lines[i]->index, file.lines[i + 1]->index);
                DEBUG(2) {
                    printf("====\n");
                    for (int i = 0; i < file.lines_cnt; ++i) {
                        printf("%s\n", file.lines[i]->string);
                    }
                }
            }
        }
    }

    return 0;
}

#endif // KCTF_GENERAL_H
