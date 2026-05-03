#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#ifndef NAME_LEN
#define NAME_LEN 32
#endif

#ifndef LEADERBOARD_FILE
#define LEADERBOARD_FILE "leaderboard.txt"
#endif

#ifndef LEADERBOARD_SIZE
#define LEADERBOARD_SIZE 5
#endif

typedef struct {
    char name[NAME_LEN];
    int score;
} LBEntry;

int load_leaderboard(LBEntry arr[], int max);
void save_leaderboard(const LBEntry arr[], int count);
void update_leaderboard(const char* name, int score);
void prompt_and_update_leaderboard(int final_score);

#ifdef __cplusplus
}
#endif

#endif /* LEADERBOARD_H */
