#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#ifndef NAME_LEN
#define NAME_LEN 32
#endif

#ifndef LEADERBOARD_SIZE
#define LEADERBOARD_SIZE 5
#endif

typedef struct {
    char name[NAME_LEN];
    int score;
} LBEntry;

/* 获取指定关卡的排行榜文件名（1-4） */
const char* get_leaderboard_file(int level);

/* 排行榜文件读写与排序维护（level: 1=经典, 2=进阶, 3=双人, 4=限时） */
int load_leaderboard(LBEntry arr[], int max, int level);
void save_leaderboard(const LBEntry arr[], int count, int level);
void update_leaderboard(const char* name, int score, int level);

/* 单人模式：游戏结束后提示输入名字并更新指定关卡排行榜，支持ESC跳过 */
void prompt_and_update_leaderboard(int final_score, int level);

/* 双人模式：提示指定玩家输入名字并记录到排行榜 */
void prompt_player_and_record(const char* player_label, int score, int level);

/* 显示指定关卡的排行榜 */
void show_leaderboard(int level);

/* 主菜单：显示全部关卡排行榜 */
void show_all_leaderboards(void);

#ifdef __cplusplus
}
#endif

#endif /* LEADERBOARD_H */
