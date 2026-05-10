/*
 * leaderboard.c - 排行榜模块实现
 * 功能：实现分模式排行榜的文件读写、排序、更新维护，
 *       以及游戏结束后的玩家名输入和前5名显示。
 *       每个关卡独立存储：leaderboard_level1~4.txt
 *       双人模式支持双方同时记录成绩。
 */
#define _CRT_SECURE_NO_WARNINGS
#include "../include/leaderboard.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>

/* 关卡名称（用于排行榜标题显示，索引0占位） */
static const char* level_names[] = {
    "Unknown",
    "Level 1: Classic",
    "Level 2: Advanced",
    "Level 3: Double Player",
    "Level 4: Time Trial"
};

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
 * get_leaderboard_file - 根据关卡编号返回对应的排行榜文件名
 * 每个模式独立存储排行榜，避免不同难度的分数混合比较。
 */
const char* get_leaderboard_file(int level) {
    switch (level) {
        case 1: return "leaderboard_level1.txt";
        case 2: return "leaderboard_level2.txt";
        case 3: return "leaderboard_level3.txt";
        case 4: return "leaderboard_level4.txt";
        default: return "leaderboard.txt";
    }
}

/*
 * load_leaderboard - 从指定关卡的排行榜文件加载数据
 * 返回值：成功读取的记录条数
 *
 * Bug修复说明：逐行读取，每行用 sscanf 解析，无法解析为 name+int 的行
 * （即表头或空行）自动跳过，兼容有无表头的文件格式。
 */
int load_leaderboard(LBEntry arr[], int max, int level) {
    const char* filename = get_leaderboard_file(level);
    FILE* fp = fopen(filename, "r");
    if (!fp) return 0;

    int count = 0;
    char line[128];
    while (count < max && fgets(line, sizeof(line), fp) != NULL) {
        char name[NAME_LEN];
        int score;
        /* sscanf 能成功解析 name + int 才视为有效数据行，
           表头"玩家名 分数"中"分数"不是整数，sscanf 返回 1，被自动跳过 */
        if (sscanf(line, "%31s %d", name, &score) == 2) {
            strncpy(arr[count].name, name, NAME_LEN - 1);
            arr[count].name[NAME_LEN - 1] = '\0';
            arr[count].score = score;
            count++;
        }
    }
    fclose(fp);
    return count;
}

/*
 * save_leaderboard - 将排行榜数据写入指定关卡的文件
 */
void save_leaderboard(const LBEntry arr[], int count, int level) {
    const char* filename = get_leaderboard_file(level);
    FILE* fp = fopen(filename, "w");
    if (!fp) return;

    /* 写入题目要求的表头（load_leaderboard 已能自动跳过） */
    fprintf(fp, "玩家名 分数\n");

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%-15s %d\n", arr[i].name, arr[i].score);
    }
    fclose(fp);
}

/*
 * update_leaderboard - 更新指定关卡排行榜（插入新记录或更新同名最高分）
 * 逻辑：1. 加载现有排行榜
 *       2. 查找同名记录，若存在且新分数更高则更新
 *       3. 若不存在则作为新记录插入
 *       4. 排序后截取前5条保存
 */
void update_leaderboard(const char* name, int score, int level) {
    LBEntry entries[LEADERBOARD_SIZE + 1];
    int count = load_leaderboard(entries, LEADERBOARD_SIZE, level);

    /* 检查是否已有同名记录，若有且当前分数更高则更新 */
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

    /* 若无同名记录，则作为新记录插入 */
    if (!found) {
        strncpy(entries[count].name, name, NAME_LEN - 1);
        entries[count].name[NAME_LEN - 1] = '\0';  /* 确保字符串以\0结尾 */
        entries[count].score = score;
        count++;
    }

    sort_leaderboard(entries, count);

    /* 只保留前5名 */
    if (count > LEADERBOARD_SIZE) {
        count = LEADERBOARD_SIZE;
    }

    save_leaderboard(entries, count, level);
}

/*
 * show_leaderboard - 显示指定关卡的排行榜
 */
void show_leaderboard(int level) {
    LBEntry entries[LEADERBOARD_SIZE];
    int count = load_leaderboard(entries, LEADERBOARD_SIZE, level);

    const char* name = (level >= 1 && level <= 4) ? level_names[level] : "Unknown";
    printf("\n  --- %s (Top %d) ---\n", name, LEADERBOARD_SIZE);
    if (count == 0) {
        printf("    (No records)\n");
    } else {
        for (int i = 0; i < count; i++) {
            printf("    %d. %-15s %d\n", i + 1, entries[i].name, entries[i].score);
        }
    }
    printf("  ----------------------------\n");
}

/*
 * input_name - 内部辅助：用 _getch() 逐字符读取名字
 * 支持 ESC 跳过、Backspace 删除、方向键过滤、不可打印字符过滤。
 * 返回值：1=成功输入有效名字, 0=跳过（ESC或空名字）
 */
static int input_name(char* name, int max_len) {
    int name_len = 0;

    while (1) {
        char ch = _getch();
        if (ch == 27) {             /* ESC 键：跳过录入 */
            name[0] = '\0';
            return 0;
        } else if (ch == -32 || ch == 0 || ch == (char)224) {
            /* 方向键前导码：读取并丢弃后续扫描码，防止乱码混入名字 */
            if (_kbhit()) _getch();
        } else if (ch == '\r' || ch == '\n') {  /* 回车：确认输入 */
            break;
        } else if (ch == '\b' || ch == 127) {   /* Backspace：删除一个字符 */
            if (name_len > 0) {
                name_len--;
                printf("\b \b");    /* 退格+覆盖空格+再退格，实现视觉删除 */
            }
        } else if (ch >= 32 && ch <= 126 && ch != ' ' && name_len < max_len - 1) {
            /* 只接受可打印的ASCII字符（排除空格和控制字符） */
            name[name_len++] = ch;
            printf("%c", ch);      /* 回显输入的字符 */
        }
    }
    name[name_len] = '\0';
    return (name_len > 0) ? 1 : 0;
}

/*
 * prompt_and_update_leaderboard - 单人模式：游戏结束后提示玩家输入名字并更新排行榜
 * 使用 _getch() 逐字符输入，支持 ESC 跳过排行榜录入直接返回菜单。
 */
void prompt_and_update_leaderboard(int final_score, int level) {
    printf("\n==== GAME OVER ====\n");
    printf("Your final score is: %d\n", final_score);
    printf("Enter your name (press ESC to skip): ");

    char name[NAME_LEN];
    if (input_name(name, NAME_LEN)) {
        update_leaderboard(name, final_score, level);
        printf("\n");
    } else {
        printf("\n(Skipped leaderboard entry)\n");
    }

    show_leaderboard(level);
}

/*
 * prompt_player_and_record - 双人模式：提示指定玩家输入名字并记录到排行榜
 * 参数：player_label - 玩家标签（如 "Player 1 (Blue)"）
 *       score - 该玩家的得分
 *       level - 关卡编号
 */
void prompt_player_and_record(const char* player_label, int score, int level) {
    printf("  %s (Score: %d)\n", player_label, score);
    printf("  Enter name (press ESC to skip): ");

    char name[NAME_LEN];
    if (input_name(name, NAME_LEN)) {
        update_leaderboard(name, score, level);
        printf("\n");
    } else {
        printf("\n  (Skipped)\n");
    }
}

/*
 * show_all_leaderboards - 主菜单：显示全部关卡排行榜
 */
void show_all_leaderboards(void) {
    for (int lv = 1; lv <= 4; lv++) {
        show_leaderboard(lv);
    }
}
