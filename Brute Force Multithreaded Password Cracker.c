// Updated main.c with formatted logging
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

#include "mta_crypt.h"
#include "mta_rand.h"

#define MAX_LEN 1024
#define MAX_CLIENTS 64

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t new_password_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t password_guessed_cond = PTHREAD_COND_INITIALIZER;

char encrypted_password[MAX_LEN];
unsigned int encrypted_len;
char real_password[MAX_LEN];
char correct_password[MAX_LEN];
int password_len = 0;
int key_len = 0;
int password_found = 0;
int timeout = -1;
int generation = 0;

long get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec;
}

void generate_printable_password(char *buffer, int len) {
    do {
        MTA_get_rand_data(buffer, len);
    } while (!isprint(buffer[0]));
    for (int i = 0; i < len; i++) {
        while (!isprint(buffer[i])) {
            buffer[i] = MTA_get_rand_char();
        }
    }
}

void log_password(const char *prefix, char *data, int len) {
    printf("%ld\t[SERVER] [%s] New password generated: ", get_time(), prefix);
    for (int i = 0; i < len; i++) putchar(isprint(data[i]) ? data[i] : '.');
    printf("\n");
}

void *encrypter_thread(void *arg) {
    int *timeout_ptr = (int *)arg;
    timeout = *timeout_ptr;

    while (1) {
        pthread_mutex_lock(&lock);
        password_found = 0;
        generation++;

        generate_printable_password(real_password, password_len);
        char key[MAX_LEN];
        key_len = password_len / 8;
        MTA_get_rand_data(key, key_len);
        MTA_encrypt(key, key_len, real_password, password_len, encrypted_password, &encrypted_len);

        printf("%ld\t[SERVER] [INFO] New password generated: ", get_time());
        for (int i = 0; i < password_len; i++) putchar(isprint(real_password[i]) ? real_password[i] : '.');
        printf(", key: ");
        for (int i = 0; i < key_len; i++) putchar(isprint(key[i]) ? key[i] : '.');
        printf(", After encryption: ");
        for (unsigned int i = 0; i < encrypted_len; i++) putchar(isprint(encrypted_password[i]) ? encrypted_password[i] : '.');
        printf("\n");

        pthread_cond_broadcast(&new_password_cond);

        if (timeout > 0) {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += timeout;
            int rc = pthread_cond_timedwait(&password_guessed_cond, &lock, &ts);
            if (!password_found) {
                printf("%ld\t[SERVER] [ERROR] No password received during the configured timeout period (%d seconds), regenerating password\n", get_time(), timeout);
            }
        } else {
            while (!password_found) {
                pthread_cond_wait(&password_guessed_cond, &lock);
            }
        }

        if (password_found) {
            printf("%ld\t[SERVER] [OK] Password decrypted successfully, is (", get_time());
            for (int i = 0; i < password_len; i++) putchar(isprint(correct_password[i]) ? correct_password[i] : '.');
            printf(")\n");
        }

        pthread_mutex_unlock(&lock);
        sleep(1);
    }
    return NULL;
}

void *decrypter_thread(void *arg) {
    int client_id = *(int *)arg;
    int local_generation = 0;
    char local_encrypted[MAX_LEN];
    unsigned int local_len;
    int iterations;

    while (1) {
        pthread_mutex_lock(&lock);
        while (local_generation == generation) {
            pthread_cond_wait(&new_password_cond, &lock);
        }

        local_generation = generation;
        memcpy(local_encrypted, encrypted_password, encrypted_len);
        local_len = encrypted_len;
        pthread_mutex_unlock(&lock);

        iterations = 0;
        while (local_generation == generation && !password_found) {
            char try_key[MAX_LEN];
            char attempt[MAX_LEN];
            unsigned int out_len;
            MTA_get_rand_data(try_key, key_len);
            iterations++;

            if (MTA_decrypt(try_key, key_len, local_encrypted, local_len, attempt, &out_len) == MTA_CRYPT_RET_OK) {
                int printable = 1;
                for (unsigned int i = 0; i < out_len; i++) {
                    if (!isprint(attempt[i])) {
                        printable = 0;
                        break;
                    }
                }

                if (printable) {
                    pthread_mutex_lock(&lock);
                    printf("%ld\t[CLIENT #%d] [INFO] After decryption(", get_time(), client_id);
                    for (unsigned int i = 0; i < out_len; i++) putchar(isprint(attempt[i]) ? attempt[i] : '.');
                    printf("), key guessed(");
                    for (int i = 0; i < key_len; i++) putchar(isprint(try_key[i]) ? try_key[i] : '.');
                    printf("), sending to server after %d iterations\n", iterations);

                    if (!password_found && memcmp(attempt, real_password, password_len) == 0) {
                        password_found = 1;
                        memcpy(correct_password, attempt, password_len);
                        pthread_cond_signal(&password_guessed_cond);
                    } else if (!password_found) {
                        printf("%ld\t[SERVER] [ERROR] Wrong password received from client #%d (", get_time(), client_id);
                        for (unsigned int i = 0; i < out_len; i++) putchar(isprint(attempt[i]) ? attempt[i] : '.');
                        printf("), should be (");
                        for (int i = 0; i < password_len; i++) putchar(isprint(real_password[i]) ? real_password[i] : '.');
                        printf(")\n");
                    }
                    pthread_mutex_unlock(&lock);
                }
            }
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int num_decrypters = 4;
    password_len = 16;
    timeout = -1;

    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--num-of-decrypters") == 0) && i + 1 < argc) {
            num_decrypters = atoi(argv[++i]);
        } else if ((strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--password-length") == 0) && i + 1 < argc) {
            password_len = atoi(argv[++i]);
        } else if ((strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--timeout") == 0) && i + 1 < argc) {
            timeout = atoi(argv[++i]);
        } else {
            fprintf(stderr, "Usage: %s [-t timeout seconds] <-n|--num-of-decrypters <number>> <-l|--password-length <length>>\n", argv[0]);
            exit(1);
        }
    }

    if (password_len % 8 != 0) {
        fprintf(stderr, "Password length must be a multiple of 8\n");
        exit(1);
    }

    if (MTA_crypt_init() != MTA_CRYPT_RET_OK) {
        fprintf(stderr, "Failed to initialize crypto lib\n");
        exit(1);
    }

    pthread_t enc_thread;
    pthread_create(&enc_thread, NULL, encrypter_thread, &timeout);

    pthread_t dec_threads[MAX_CLIENTS];
    int ids[MAX_CLIENTS];
    for (int i = 0; i < num_decrypters; i++) {
        ids[i] = i;
        pthread_create(&dec_threads[i], NULL, decrypter_thread, &ids[i]);
    }

    pthread_join(enc_thread, NULL);
    for (int i = 0; i < num_decrypters; i++) {
        pthread_cancel(dec_threads[i]);
        pthread_join(dec_threads[i], NULL);
    }

    return 0;
}
