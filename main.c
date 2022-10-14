#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

// Да, я использовала глобальные переменные, тк мне так удобно.
char *pattern_var;
char *absolute_path;

// Структура элемента двусвязного списка компонент относительного пути.
typedef struct path_part {
    char * part;
    struct path_part *next;
    struct path_part *prev;
}   path_part;

// Вспомогательная структура хранящая в себе начало и конец двусвязного списка.
typedef struct path {
    path_part *first;
    path_part *last;
}   path;

// Добавление элемента elem в конец двусвязного списка компонент относительного пути.
void push_back(path *rp, path_part *elem) {
    if (!elem || !rp) {
        return;
    }
    if (rp->last) {
        rp->last->next = elem;
        elem->prev = rp->last;
    } else {
        rp->first = elem;
    }
    rp->last = elem;
}

// Удаление последнего элемента из двусвязного списка компонент относительного пути.
void pop_back(path *rp) {
    path_part *newlast = NULL;
    if (rp->last) {
        newlast = rp->last->prev;
        if (newlast) {
            newlast->next = NULL;
        } else {
            rp->first = NULL;
        }
        free(rp->last->part);
        free(rp->last);
    }
    rp->last = newlast;
}

// Очистка памяти двусвязного списка компонент относительного пути.
void clear(path *rp) {
    while (rp->first) {
        pop_back(rp);
    }
}

// Создается и возвращается новый компонент относительного пути с копированием контента.
// Возвращается NULL, если возникли проблемы с выделением памяти.
path_part *new_part(const char *part) {
    path_part *res;
    if (!(res = (path_part *)malloc(sizeof *res))) {
        return NULL;
    }
    char *content;
    if (!(content = strdup(part))) {
        free(res);
        return NULL;
    }
    res->part = content;
    res->prev = NULL;
    res->next = NULL;
    return res;
}

// Функция возвращает строку - относительный путь до файла. Или NULL, если возникли проблемы с выделением памяти.
char *get_relative_path(path *rp, char *dirname) {
    char *result;
    if (!(result = (char *)malloc((PATH_MAX + 1) * sizeof *result)))
        return NULL;
    result[0] = '\0';
    if (absolute_path)
        result = strcat(result, absolute_path);
    path_part *ptr = rp->first;
    while (ptr != NULL) {
        result = strcat(result, ptr->part);
        result = strcat(result, "/");
        ptr = ptr->next;
    }
    result = strcat(result, dirname);
    return result;
}

// Вывод подсказки, срабатывает при неправильном количестве аргументов.
void print_usage() {
    printf("usage: ff filename (or \"pattern_name\") [directory]\n");
}

// Эта функция возвращает 1, если str соответствует pattern и 0 в обратном случае.
// Рекурсивно вызывает саму себя, смещая указатели начала строк.
int pattern_match(char *pattern, char *str) {
    while (*pattern != '\0' && *str != '\0') {
        if (*pattern == '*') {
            if (pattern_match(pattern, str + sizeof *str))
                return 1;
        } else if (*pattern != '?' && *pattern != *str) {
            return 0;
        }
        pattern += sizeof *pattern;
        str += sizeof *str;
    }
    return *pattern == '\0' && *str == '\0' ? 1 : 0;
}

// Просто текстовый вывод информации об ошибках, которые могут произойти при использовании функции opendir.
void print_error() {
    printf("--------------------------------------------------------------------------------\n");
    switch (errno) {
        case ENOENT:
            printf("ERROR: No such directory exists!\n");
            break;
        case EACCES:
            printf("ERROR: Read permission is denied!\n");
            break;
        case EMFILE:
            printf("ERROR: The process has too many files open!\n");
            break;
        case ENFILE:
            printf("ERROR: Cannot support any additional open files at the moment!\n");
            break;
        case ENOMEM:
            printf("ERROR: Not enough memory available!\n");
            break;
        default:
            printf("ERROR: An unexpected error occurred!\n");
    }
    printf("--------------------------------------------------------------------------------\n");
}

// Рекурсивный обход переданной директории d.
// В структуру rp записываются указатели на первый и последний компонент относительного пути,
// Все попадающиеся файлы проверяются на соответствие паттерну и выводятся или игнорируются.
// Директории: игнорируются текущие и родительские, остальные - рекурсивно обходятся этой функцией,
// в двусвязный список компонент пути добавляется новая компонента равная d_name.
// При возникновении ошибок функция должна нормально завершиться, стек вызовов развернется и выполнится вся очистка памяти,
// запланированная после вызова рекурсии.
void print_dir_content(DIR *d, path *rp) {
    struct dirent *dir;
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG && pattern_match(pattern_var, dir->d_name) == 1) {
                char *path = get_relative_path(rp, dir->d_name);
                printf("%s\n", path);
                if (path)
                    free(path);
            } else if (dir->d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
                push_back(rp, new_part(dir->d_name));
                char *path = get_relative_path(rp, "");
                DIR *other = opendir(path);
                if (path)
                    free(path);
                print_dir_content(other, rp);
                pop_back(rp);
            }
        }
        closedir(d);
    } else {
        print_error();
    }
}

// Точка входа. Здесь инициализируются глобальные переменные, проверяется количество аргументов,
// запускается функция рекурсивного обхода.
int main(int argc, char** argv) {
    if (argc != 2 && argc != 3) {
        print_usage();
    } else {
        DIR *d;
        absolute_path = argc == 2 ? NULL : argv[2];
        d = argc == 2 ? opendir(".") : opendir(argv[2]);
        pattern_var = argv[1];
        path rp;
        rp.first = NULL;
        rp.last = NULL;
        print_dir_content(d, &rp);
        clear(&rp);
    }
    return 0;
}
