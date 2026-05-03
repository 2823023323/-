#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include "../include/game_shared.h"
#include "../include/level1.h"
#include "../include/leaderboard.h"

void show_main_menu() {
    clear_screen();
    set_color(11); // 亮青色
    printf("=========================================\n");
    printf("        TERMINAL SNAKE GAME              \n");
    printf("=========================================\n\n");
    set_color(15);
    printf("   1. Start Classic Mode (Level 1)\n");
    printf("   2. View Leaderboard\n");
    printf("   3. Exit\n\n");
    printf("=========================================\n");
    printf("Please press 1, 2 or 3 to select...\n");
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
                        clear_screen();
                        set_color(15);
                        printf("\n--- LEADERBOARD ---\n");
                        LBEntry entries[LEADERBOARD_SIZE];
                        int count = load_leaderboard(entries, LEADERBOARD_SIZE);
                        if (count == 0) {
                            printf("No records found.\n");
                        } else {
                            for (int i = 0; i < count; i++) {
                                printf("%d. %-15s %d\n", i + 1, entries[i].name, entries[i].score);
                            }
                        }
                        printf("-------------------\n");
                        printf("\nPress any key to return to main menu...");
                        _getch(); // 等待按键
                        choice_made = 1;
                        break;
                    case '3':
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
