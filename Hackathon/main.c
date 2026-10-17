#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

static atomic_int stop_test = 0;
static atomic_uint_fast64_t operations = 0;

static void handle_signal(int sig)
{
    (void)sig;
    atomic_store(&stop_test, 1);
}

static void *cpu_stress(void *arg)
{
    uint64_t x = (uint64_t)(uintptr_t)arg + 1;

    while (!atomic_load(&stop_test)) {
        for (int i = 0; i < 100000; i++)
            x = x * 1664525ULL + 1013904223ULL;

        atomic_fetch_add(&operations, 100000);
    }

    return NULL;
}

static int parse_long(const char *s, long *out)
{
    char *end = NULL;
    errno = 0;
    long value = strtol(s, &end, 10);

    if (errno != 0 || end == s || *end != '\0')
        return 0;

    *out = value;
    return 1;
}

int main(int argc, char *argv[])
{
    if (argc != 4) {
        printf("Usage: %s <seconds> <cpu_threads> <memory_MB>\n", argv[0]);
        printf("Example: %s 30 4 512\n", argv[0]);
        return 1;
    }

    long duration, thread_count, memory_mb;
    long online_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    struct sysinfo info;

    if (!parse_long(argv[1], &duration) ||
        !parse_long(argv[2], &thread_count) ||
        !parse_long(argv[3], &memory_mb) ||
        duration < 1 || duration > 3600 ||
        thread_count < 1 || thread_count > online_cpus ||
        memory_mb < 0 || memory_mb > 1048576 ||
        sysinfo(&info) != 0) {
        fprintf(stderr, "Invalid settings or system information unavailable.\n");
        fprintf(stderr, "Duration: 1-3600 seconds; threads: 1-%ld; memory: 0-1048576 MB.\n",
                online_cpus);
        return 1;
    }

    unsigned long long available_mb =
        (unsigned long long)info.freeram * info.mem_unit /
        (1024ULL * 1024ULL);

    if ((unsigned long long)memory_mb > available_mb / 2) {
        fprintf(stderr, "Requested memory is too high. Current reported free RAM: %llu MB.\n",
                available_mb);
        fprintf(stderr, "Choose no more than half of that amount.\n");
        return 1;
    }

    size_t bytes = (size_t)memory_mb * 1024ULL * 1024ULL;
    unsigned char *memory = NULL;

    if (bytes > 0) {
        memory = malloc(bytes);
        if (memory == NULL) {
            perror("malloc");
            return 1;
        }
        for (size_t i = 0; i < bytes; i++)
            memory[i] = (unsigned char)(i % 251);
    }

    pthread_t *workers = calloc((size_t)thread_count, sizeof(*workers));
    if (workers == NULL) {
        perror("calloc");
        free(memory);
        return 1;
    }

    struct sigaction sa = {0};
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    printf("CPU and Memory Stress Testing Tool\n");
    printf("----------------------------------\n");
    printf("CPU threads : %ld\n", thread_count);
    printf("Memory      : %ld MB\n", memory_mb);
    printf("Duration    : %ld seconds\n", duration);
    printf("Press Ctrl+C to stop early.\n\n");

    int created = 0;
    for (long i = 0; i < thread_count; i++) {
        int rc = pthread_create(&workers[i], NULL, cpu_stress,
                                (void *)(uintptr_t)i);
        if (rc != 0) {
            fprintf(stderr, "Could not create worker thread (%d).\n", rc);
            atomic_store(&stop_test, 1);
            break;
        }
        created++;
    }

    time_t start = time(NULL);
    time_t last_report = start;
    uint64_t checksum = 0;

    while (!atomic_load(&stop_test) && time(NULL) - start < duration) {
        if (memory != NULL) {
            for (size_t i = 0; i < bytes; i += 4096)
                memory[i] = (unsigned char)(memory[i] + 1);

            for (size_t i = 0; i < bytes; i += 64)
                checksum += memory[i];
        }

        time_t now = time(NULL);
        if (now != last_report) {
            printf("\rElapsed: %lld/%ld sec | CPU operations: %llu",
                   (long long)(now - start), duration,
                   (unsigned long long)atomic_load(&operations));
            fflush(stdout);
            last_report = now;
        }
        if (bytes == 0)
            usleep(1000);
    }

    atomic_store(&stop_test, 1);
    for (int i = 0; i < created; i++)
        pthread_join(workers[i], NULL);

    printf("\n\nTest %s.\n",
           (time(NULL) - start >= duration) ? "completed" : "stopped early");
    printf("Elapsed seconds : %lld\n", (long long)(time(NULL) - start));
    printf("CPU operations  : %llu\n",
           (unsigned long long)atomic_load(&operations));
    printf("Memory checksum : %llu\n", (unsigned long long)checksum);

    free(workers);
    free(memory);
    return created == thread_count ? 0 : 1;
}
