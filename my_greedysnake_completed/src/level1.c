/*
 * level1.c - 第一关（经典模式）游戏主流程
 * 功能：组装 game_shared、level_common、game_ui、leaderboard 模块，
 *       实现完整的经典贪吃蛇游戏循环。
 * 流程：初始化 -> 主循环(输入->移动->碰撞->吃食->绘制) -> 排行榜 -> 重启
 */
#define _CRT_SECURE_NO_WARNINGS
#include "../include/level1.h"
#include "../include/level_common.h"
#include "../include/game_shared.h"
#include "../include/game_ui.h"
#include "../include/leaderboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

void level1_run(void) {
    int restart = 1;
    
    while (restart) {
        Snake snake;
        Food food;
        int game_over = 0;
        
        hide_cursor();
        clear_screen();
        
        // 关卡说明界面（让老师和玩家在开始前了解操作方式）
        set_color(11);
        printf("\n  ===== LEVEL 1: CLASSIC MODE =====\n\n");
        set_color(15);
        printf("  Controls: Arrow Keys / WASD\n");
        printf("  Rules:\n");
        printf("    - Eat food ($) to grow and score\n");
        printf("    - Avoid walls (#) and yourself\n");
        printf("    - Press ESC to quit\n\n");
        set_color(14);
        printf("  Press any key to start...\n");
        set_color(7);
        _getch();
        clear_screen();  // 清除说明界面，准备绘制游戏画面
        snake_init(&snake, WIDTH / 2, HEIGHT / 2);
        place_food_safe(&food, &snake, FOOD_NORMAL);
        
        unsigned long last_move_time = get_tick_ms();
        unsigned long move_interval = 150;  // 移动间隔（毫秒）
        
        // 绘制初始画面（修复：进入游戏后立即可见，无需等待首次移动）
        draw_board(&snake, &food);
        
        // ===== 游戏主循环 =====
        while (snake.alive && !game_over) {
            unsigned long current_time = get_tick_ms();
            
            // 1. 处理键盘输入（无阻塞）
            process_input(&snake, &game_over);
            
            // 2. 判断是否到达移动间隔
            if (current_time - last_move_time >= move_interval) {
                // 3. 移动蛇（头增尾删）
                snake_move(&snake);
                last_move_time = current_time;
                
                // 4. 碰撞检测（墙壁 + 自身）
                if (check_wall_collision(&snake) || check_self_collision(&snake)) {
                    snake.alive = 0;
                    break;
                }
                
                // 5. 吃食物判断
                if (snake.x[0] == food.x && snake.y[0] == food.y) {
                    snake.score++;
                    snake_grow(&snake);
                    // 尝试生成新食物，若地图已满则判定胜利
                    if (!place_food_safe(&food, &snake, FOOD_NORMAL)) {
                        set_cursor_pos(0, HEIGHT + 2);
                        set_color(10);
                        printf("\n==== YOU WIN! MAP CLEARED! ====\n");
                        snake.alive = 0;
                        break;
                    }
                }
                
                // 6. 绘制画面
                draw_board(&snake, &food);
            }
            
            // 7. 短暂延时，降低CPU占用
            Sleep(10);
        }
        
        // ===== 游戏结束处理 =====
        set_cursor_pos(0, HEIGHT + 2);
        set_color(15);
        prompt_and_update_leaderboard(snake.score);
        
        // 重启询问
        printf("\nRestart? (y/n): ");
        char choice;
        while (1) {
            if (_kbhit()) {
                choice = _getch();
                if (choice == 'y' || choice == 'Y') {
                    restart = 1;
                    break;
                } else if (choice == 'n' || choice == 'N') {
                    restart = 0;
                    break;
                }
            }
        }
    }
}
