/*
 * leaderboard.c - 排行榜模块实现
 * 功能：实现排行榜的文件读写、排序、更新维护，
 *       以及游戏结束后的玩家名输入和前5名显示。
 *       排行榜数据以"玩家名 分数"格式存储在 leaderboard.txt 中。
 */
#define _CRT_SECURE_NO_WARNINGS
#include "../include/leaderboard.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * sort_leaderboard - 将排行榜记录按分数降序排序（选择排序）
 * 注意：static 关键字限制本函数仅在本文件内可用，体现封装原则
 */
static void sort_leaderboard(LBEntry arr[], int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (arr[i].score < arr[j].score) {
                LBEntry temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
    }
}

/*
 * load_leaderboard - 从文件加载排行榜数据
 * 返回值：成功读取的记录条数
 */
int load_leaderboard(LBEntry arr[], int max) {
    FILE* fp = fopen(LEADERBOARD_FILE, "r");
    if (!fp) return 0;
    
    int count = 0;
    // 使用 %31s 限制读取长度，防止缓冲区溢出（NAME_LEN=32，留1位给\0）
    while (count < max && fscanf(fp, "%31s %d", arr[count].name, &arr[count].score) == 2) {
        count++;
    }
    fclose(fp);
    return count;
}

/*
 * save_leaderboard - 将排行榜数据写入文件
 */
void save_leaderboard(const LBEntry arr[], int count) {
    FILE* fp = fopen(LEADERBOARD_FILE, "w");
    if (!fp) return;
    
    // 写入题目要求的表头
    fprintf(fp, "玩家名 分数\n");
    
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%-15s %d\n", arr[i].name, arr[i].score);
    }
    fclose(fp);
}

/*
 * update_leaderboard - 更新排行榜（插入新记录或更新同名最高分）
 * 逻辑：1. 加载现有排行榜
 *       2. 查找同名记录，若存在且新分数更高则更新
 *       3. 若不存在则作为新记录插入
 *       4. 排序后截取前5条保存
 */
void update_leaderboard(const char* name, int score) {
    LBEntry entries[LEADERBOARD_SIZE + 1];
    int count = load_leaderboard(entries, LEADERBOARD_SIZE);
    
    // 检查是否已有同名记录，若有且当前分数更高则更新
    int found = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(entries[i].name, name) == 0) {
            if (score > entries[i].score) {
                entries[i].score = score;
            }
            found = 1;
            break;
        }
    }
    
    // 若无同名记录，则作为新记录插入
    if (!found) {
        strncpy(entries[count].name, name, NAME_LEN - 1);
        entries[count].name[NAME_LEN - 1] = '\0';  // 确保字符串以\0结尾
        entries[count].score = score;
        count++;
    }
    
    sort_leaderboard(entries, count);
    
    // 只保留前5名
    if (count > LEADERBOARD_SIZE) {
        count = LEADERBOARD_SIZE;
    }
    
    save_leaderboard(entries, count);
}

/*
 * prompt_and_update_leaderboard - 游戏结束后提示玩家输入名字并更新排行榜
 */
void prompt_and_update_leaderboard(int final_score) {
    printf("\n==== GAME OVER ====\n");
    printf("Your final score is: %d\n", final_score);
    
    char name[NAME_LEN];
    printf("Enter your name (no spaces): ");
    // 使用 %31s 限制输入长度，防止缓冲区溢出
    if (scanf("%31s", name) == 1) {
        update_leaderboard(name, final_score);
    }
    // 清空输入缓冲区，防止残留字符影响后续 _getch()
    while (getchar() != '\n');
    
    // 显示排行榜
    LBEntry entries[LEADERBOARD_SIZE];
    int count = load_leaderboard(entries, LEADERBOARD_SIZE);
    printf("\n--- LEADERBOARD (Top %d) ---\n", LEADERBOARD_SIZE);
    for (int i = 0; i < count; i++) {
        printf("%d. %-15s %d\n", i + 1, entries[i].name, entries[i].score);
    }
    printf("---------------------------\n");
}
