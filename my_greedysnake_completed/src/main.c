/*
 * main.c - 程序入口与主菜单
 * 功能：初始化随机种子、隐藏光标，
 *       显示彩色主菜单并响应用户选择，
 *       进入不同关卡或查看排行榜。
 */
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include "../include/game_shared.h"
#include "../include/level1.h"
#include "../include/level2.h"
#include "../include/level3.h"
#include "../include/level4.h"
#include "../include/leaderboard.h"

void show_main_menu() {
    clear_screen();
    set_color(11);
    printf("=========================================\n");
    printf("        TERMINAL SNAKE GAME              \n");
    printf("=========================================\n\n");
    set_color(15);
    printf("   1. Classic Mode    (Level 1)\n");
    printf("   2. Advanced Mode   (Level 2)\n");
    printf("   3. Double Player   (Level 3)\n");
    printf("   4. Time Trial Mode (Level 4)\n");
    printf("   5. View Leaderboard\n");
    printf("   6. Exit\n\n");
    set_color(11);
    printf("=========================================\n");
    set_color(7);
    printf("Press 1-6 to select...\n");
}

int main() {
    srand((unsigned int)time(NULL));
    hide_cursor();
    
    while (1) {
        show_main_menu();
        
        int choice_made = 0;
        while (!choice_made) {
            if (_kbhit()) {
                char choice = _getch();
                switch (choice) {
                    case '1':
                        level1_run();
                        choice_made = 1;
                        break;
                    case '2':
                        level2_run();
                        choice_made = 1;
                        break;
                    case '3':
                        level3_run();
                        choice_made = 1;
                        break;
                    case '4':
                        level4_run();
                        choice_made = 1;
                        break;
                    case '5': {
                        clear_screen();
                        set_color(15);
                        printf("\n--- LEADERBOARD (Top %d) ---\n", LEADERBOARD_SIZE);
                        LBEntry entries[LEADERBOARD_SIZE];
                        int count = load_leaderboard(entries, LEADERBOARD_SIZE);
                        if (count == 0) {
                            printf("No records found.\n");
                        } else {
                            for (int i = 0; i < count; i++) {
                                printf("%d. %-15s %d\n", i + 1, entries[i].name, entries[i].score);
                            }
                        }
                        printf("---------------------------\n");
                        printf("\nPress any key to return...");
                        _getch();
                        choice_made = 1;
                        break;
                    }
                    case '6':
                        clear_screen();
                        set_color(15);
                        printf("Goodbye!\n");
                        return 0;
                }
            }
        }
    }
    
    return 0;
}
