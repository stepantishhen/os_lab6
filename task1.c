#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

// Функция для получения текущего времени в формате "часы : минуты : секунды : миллисекунды"
void get_current_time(char *buffer, size_t buffer_size) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts); // Получаем текущее время

    struct tm *tm_info = localtime(&ts.tv_sec); // Преобразуем время в структуру tm
    strftime(buffer, buffer_size, "%H : %M : %S", tm_info); // Форматируем время

    // Добавляем миллисекунды
    snprintf(buffer + strlen(buffer), buffer_size - strlen(buffer), " : %03ld", ts.tv_nsec / 1000000);
}

int main() {
    pid_t pid1, pid2;

    // Первый вызов fork()
    pid1 = fork();

    if (pid1 < 0) {
        // Ошибка при создании процесса
        perror("Ошибка при вызове fork()");
        exit(EXIT_FAILURE);
    } else if (pid1 == 0) {
        // Дочерний процесс 1
        char time_buffer[64];
        get_current_time(time_buffer, sizeof(time_buffer));

        printf("Дочерний процесс 1: pid = %d, ppid = %d, время = %s\n",
               getpid(), getppid(), time_buffer);
        exit(EXIT_SUCCESS); // Завершаем дочерний процесс 1
    } else {
        // Родительский процесс
        // Второй вызов fork()
        pid2 = fork();

        if (pid2 < 0) {
            // Ошибка при создании процесса
            perror("Ошибка при вызове fork()");
            exit(EXIT_FAILURE);
        } else if (pid2 == 0) {
            // Дочерний процесс 2
            char time_buffer[64];
            get_current_time(time_buffer, sizeof(time_buffer));

            printf("Дочерний процесс 2: pid = %d, ppid = %d, время = %s\n",
                   getpid(), getppid(), time_buffer);
            exit(EXIT_SUCCESS); // Завершаем дочерний процесс 2
        } else {
            // Родительский процесс
            char time_buffer[64];
            get_current_time(time_buffer, sizeof(time_buffer));

            printf("Родительский процесс: pid = %d, ppid = %d, время = %s\n",
                   getpid(), getppid(), time_buffer);

            // Ожидаем завершения дочерних процессов
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);

            // Выполняем команду ps -x
            printf("\nВыполняем команду ps -x:\n");
            system("ps -x");

            exit(EXIT_SUCCESS); // Завершаем родительский процесс
        }
    }
}
