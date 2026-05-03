#include "../include/level1.h"
#include "../include/level_common.h"
#include "../include/game_shared.h"
#include "../include/game_ui.h"
#include "../include/leaderboard.h"
#include <stdio.h>
#include <stdlib.h>

void level1_run(void) {
    int restart = 1;
    
    while (restart) {
        Snake snake;
        Food food;
        int game_over = 0;
        
        hide_cursor();
        snake_init(&snake, WIDTH / 2, HEIGHT / 2);
        place_food_safe(&food, &snake, FOOD_NORMAL);
        
        unsigned long last_move_time = get_tick_ms();
        int move_interval = 150; // 毫秒
        
        // 主循环
        while (snake.alive && !game_over) {
            unsigned long current_time = get_tick_ms();
            
            // 1. 处理输入
            process_input(&snake, &game_over);
            
            // 2. 定时移动
            if (current_time - last_move_time >= move_interval) {
                snake_move(&snake);
                last_move_time = current_time;
                
                // 3. 碰撞检测
                if (check_wall_collision(&snake) || check_self_collision(&snake)) {
                    snake.alive = 0;
                    break;
                }
                
                // 4. 吃食物判断
                if (snake.x[0] == food.x && snake.y[0] == food.y) {
                    snake.score++;
                    snake_grow(&snake);
                    place_food_safe(&food, &snake, FOOD_NORMAL);
                }
                
                // 5. 绘制画面
                draw_board(&snake, &food);
            }
            
            Sleep(10); // 短暂延时，减少 CPU 占用
        }
        
        // 游戏结束
        set_cursor_pos(0, HEIGHT + 2);
        set_color(15);
        prompt_and_update_leaderboard(snake.score);
        
        printf("\nDo you want to restart? (y/n): ");
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
