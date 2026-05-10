/*
 * level4.c - 第四关（限时生存模式）游戏主流程
 * 功能：在60秒限时内尽可能多地吃食物，同时躲避多个快速移动的障碍物。
 *       没有任何固定障碍物或毒药，纯粹考验走位与反应速度。
 */
#define _CRT_SECURE_NO_WARNINGS
#include "../include/level4.h"
#include "../include/level_common.h"
#include "../include/game_shared.h"
#include "../include/game_ui.h"
#include "../include/leaderboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#define MAX_MOB 3

void level4_run(void) {
    int restart = 1;
    
    while (restart) {
        Snake snake;
        Food food;
        int game_over = 0;
        int win = 0;
        
        // 移动障碍物初始化 (3个不同高度和速度的障碍物)
        int mob_x[MAX_MOB] = {WIDTH / 2, WIDTH / 3, WIDTH * 2 / 3};
        int mob_y[MAX_MOB] = {HEIGHT / 4, HEIGHT / 2, HEIGHT * 3 / 4};
        int mob_dir[MAX_MOB] = {1, -1, 1};
        unsigned long mob_intervals[MAX_MOB] = {80, 100, 120}; // 移动速度不同
        unsigned long last_mob_move[MAX_MOB];
        
        hide_cursor();
        clear_screen();
        
        // 关卡说明界面
        set_color(11);
        printf("\n  ===== LEVEL 4: TIME TRIAL MODE =====\n\n");
        set_color(15);
        printf("  Controls: Arrow Keys / WASD\n\n");
        printf("  Objective:\n");
        printf("    Survive 60 seconds and score as high as possible!\n\n");
        set_color(12); printf("    O "); set_color(15); printf("- Moving obstacles (instant death)\n");
        set_color(10); printf("    $ "); set_color(15); printf("- Food (+1 score, +1 length)\n\n");
        printf("  3 obstacles patrol at different speeds.\n");
        printf("  Dodge them and eat to score!\n\n");
        set_color(6);
        printf("  * WARNING: Obstacles speed up every 15s!\n\n");
        set_color(14);
        printf("  Press any key to start...\n");
        set_color(7);
        _getch();
        clear_screen();
        // 蛇出生在地图中央偏下，避免与移动障碍物#2（y=HEIGHT/2）同行
        snake_init(&snake, WIDTH / 2, HEIGHT / 2 + 2);
        // 生成食物（确保不在移动障碍物的当前位置上）
        /* Bug修复：place_food_safe返回0表示地图已满，
           原版忽略返回值导致 fok 永远为0，死循环。
           现在若放置失败直接跳出，视为胜利（蛇占满地图）。 */
        {
            int food_ok = 0;
            int food_attempts = 0;
            while (!food_ok && food_attempts < 1000) {
                food_attempts++;
                if (!place_food_safe(&food, &snake, FOOD_NORMAL)) {
                    /* 地图已满，无法放食物，触发胜利 */
                    snake.alive = 0;
                    win = 1;
                    break;
                }
                food_ok = 1;
                for (int i = 0; i < MAX_MOB; i++) {
                    if (food.x == mob_x[i] && food.y == mob_y[i]) {
                        food_ok = 0; break;
                    }
                }
            }
        }
        
        // 限时模式初始化
        unsigned long start_time = get_tick_ms();
        int time_limit = 60;    // 60秒生存极限
        int time_left = time_limit;
        
        unsigned long last_move_time = get_tick_ms();
        unsigned long move_interval = 150;
        
        for(int i=0; i<MAX_MOB; i++) {
            last_mob_move[i] = get_tick_ms();
        }
        
        // 绘制初始画面
        draw_board_level4(&snake, &food, mob_x, mob_y, MAX_MOB, time_left);
        
        // ===== 游戏主循环 =====
        while (snake.alive && !game_over) {
            unsigned long current_time = get_tick_ms();
            
            // 更新倒计时
            time_left = time_limit - (current_time - start_time) / 1000;
            if (time_left <= 0) {
                time_left = 0;
                win = 1;
                break; // 倒计时结束，存活胜利！
            }
            
            // 障碍物随时间加速：每过15秒，速度提升20%（间隔缩短）
            {
                int elapsed = time_limit - time_left;
                int speed_level = elapsed / 15;  // 0-15s: 0, 15-30s: 1, 30-45s: 2, 45-60s: 3
                for (int i = 0; i < MAX_MOB; i++) {
                    // 用原始速度乘以 0.8^speed_level 计算当前间隔
                    unsigned long base = (i == 0) ? 80 : (i == 1) ? 100 : 120;
                    for (int s = 0; s < speed_level; s++) {
                        base = base * 4 / 5;  // 每级提速20%
                    }
                    if (base < 30) base = 30;  // 最低30ms，防止过快
                    mob_intervals[i] = base;
                }
            }
            
            process_input(&snake, &game_over);
            
            // 移动障碍物逻辑
            int mob_moved = 0;
            for (int i = 0; i < MAX_MOB; i++) {
                if (current_time - last_mob_move[i] >= mob_intervals[i]) {
                    mob_x[i] += mob_dir[i];
                    // 碰到墙壁反弹
                    if (mob_x[i] <= 1) {
                        mob_x[i] = 2; mob_dir[i] = 1;
                    } else if (mob_x[i] >= WIDTH - 2) {
                        mob_x[i] = WIDTH - 3; mob_dir[i] = -1;
                    }
                    last_mob_move[i] = current_time;
                    mob_moved = 1;
                    
            /* Bug修复：原版 break 只退出内层 j 循环，蛇死后外层 i 循环仍继续。
               改用 mob_killed 标志，内层 break 后立即跳出外层循环。 */
            int mob_killed = 0;
            for (int j = 0; j < snake.length; j++) {
                if (snake.x[j] == mob_x[i] && snake.y[j] == mob_y[i]) {
                    snake.alive = 0;
                    mob_killed = 1;
                    break;
                }
            }
            if (mob_killed) break;  /* 退出外层 i 循环 */
                }
            }
            if (!snake.alive) break;
            
            if (current_time - last_move_time >= move_interval) {
                snake_move(&snake);
                last_move_time = current_time;
                
                // 碰撞检测：墙壁 + 自身
                if (check_wall_collision(&snake) || check_self_collision(&snake)) {
                    snake.alive = 0; break;
                }
                
                // 碰撞检测：移动障碍物
                for (int i = 0; i < MAX_MOB; i++) {
                    if (snake.x[0] == mob_x[i] && snake.y[0] == mob_y[i]) {
                        snake.alive = 0; break;
                    }
                }
                if (!snake.alive) break;
                
                // 吃食物检测
                if (snake.x[0] == food.x && snake.y[0] == food.y) {
                    snake.score++;
                    snake_grow(&snake);
                    // 重新生成食物（避开移动障碍物位置）
                    /* Bug修复：同上，食物重生时也检查place_food_safe返回值 */
                    {
                        int fok = 0;
                        int fok_attempts = 0;
                        while (!fok && fok_attempts < 1000) {
                            fok_attempts++;
                            if (!place_food_safe(&food, &snake, FOOD_NORMAL)) break;
                            fok = 1;
                            for (int i = 0; i < MAX_MOB; i++) {
                                if (food.x == mob_x[i] && food.y == mob_y[i]) {
                                    fok = 0; break;
                                }
                            }
                        }
                        if (!fok) {
                            snake.alive = 0;
                            win = 1;
                            break;
                        }
                    }
                }
                
                if (snake.alive) {
                    draw_board_level4(&snake, &food, mob_x, mob_y, MAX_MOB, time_left);
                }
            } else if (mob_moved && snake.alive) {
                // 如果仅仅是障碍物移动了，也需要重绘以更新障碍物位置
                draw_board_level4(&snake, &food, mob_x, mob_y, MAX_MOB, time_left);
            }
            
            Sleep(10);
        }
        
        // ===== 游戏结束处理 =====
        set_cursor_pos(0, HEIGHT + 2);
        set_color(15);
        if (win) {
            set_color(10);
            printf("\n==== CONGRATULATIONS! YOU SURVIVED 60 SECONDS! ====\n");
            set_color(15);
        } else {
            printf("\n==== YOU DIED! YOU SURVIVED %d SECONDS ====\n", 60 - time_left);
        }
        prompt_and_update_leaderboard(snake.score);
        
        printf("\nRestart Level 4? (y/n): ");
        char choice;
        while (1) {
            if (_kbhit()) {
                choice = _getch();
                if (choice == 'y' || choice == 'Y') {
                    restart = 1; break;
                } else if (choice == 'n' || choice == 'N') {
                    restart = 0; break;
                }
            }
        }
    }
}
