#include "../include/leaderboard.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 将记录按分数降序排序
void sort_leaderboard(LBEntry arr[], int count) {
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

int load_leaderboard(LBEntry arr[], int max) {
    FILE* fp = fopen(LEADERBOARD_FILE, "r");
    if (!fp) return 0;
    
    int count = 0;
    while (count < max && fscanf(fp, "%s %d", arr[count].name, &arr[count].score) == 2) {
        count++;
    }
    fclose(fp);
    return count;
}

void save_leaderboard(const LBEntry arr[], int count) {
    FILE* fp = fopen(LEADERBOARD_FILE, "w");
    if (!fp) return;
    
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s %d\n", arr[i].name, arr[i].score);
    }
    fclose(fp);
}

void update_leaderboard(const char* name, int score) {
    LBEntry entries[LEADERBOARD_SIZE + 1];
    int count = load_leaderboard(entries, LEADERBOARD_SIZE);
    
    // 检查是否已经是同名记录且当前分数更高
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
    
    // 如果没有找到同名记录，则作为新记录插入
    if (!found) {
        strcpy(entries[count].name, name);
        entries[count].score = score;
        count++;
    }
    
    sort_leaderboard(entries, count);
    
    if (count > LEADERBOARD_SIZE) {
        count = LEADERBOARD_SIZE;
    }
    
    save_leaderboard(entries, count);
}

void prompt_and_update_leaderboard(int final_score) {
    printf("\n==== GAME OVER ====\n");
    printf("Your final score is: %d\n", final_score);
    
    char name[NAME_LEN];
    printf("Enter your name (no spaces): ");
    if (scanf("%31s", name) == 1) {
        update_leaderboard(name, final_score);
    }
    
    // 显示排行榜
    LBEntry entries[LEADERBOARD_SIZE];
    int count = load_leaderboard(entries, LEADERBOARD_SIZE);
    printf("\n--- LEADERBOARD ---\n");
    for (int i = 0; i < count; i++) {
        printf("%d. %-15s %d\n", i + 1, entries[i].name, entries[i].score);
    }
    printf("-------------------\n");
}
