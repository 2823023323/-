#include "../include/level2.h"
#include "../include/level_common.h"
#include "../include/game_shared.h"
#include "../include/game_ui.h"
#include "../include/leaderboard.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_FOODS 4
#define MAX_OBS 10

// 生成随机且不重合的障碍物
void init_obstacles(int obs_x[], int obs_y[], int obs_count, const Snake* s) {
    for (int i = 0; i < obs_count; i++) {
        int valid = 0;
        while (!valid) {
            obs_x[i] = 2 + rand() % (WIDTH - 4);
            obs_y[i] = 2 + rand() % (HEIGHT - 4);
            valid = 1;
            
            // 不能和蛇重合
            for (int j = 0; j < s->length; j++) {
                if (s->x[j] == obs_x[i] && s->y[j] == obs_y[i]) {
                    valid = 0;
                    break;
                }
            }
            // 不能和其他障碍物重合
            for (int j = 0; j < i; j++) {
                if (obs_x[j] == obs_x[i] && obs_y[j] == obs_y[i]) {
                    valid = 0;
                    break;
                }
            }
        }
    }
}

// 避开障碍物放置食物
void place_food_safe_level2(Food* f, const Snake* s, int type, const int obs_x[], const int obs_y[], int obs_count) {
    int valid = 0;
    while (!valid) {
        f->x = 1 + rand() % (WIDTH - 2);
        f->y = 1 + rand() % (HEIGHT - 2);
        valid = 1;
        
        for (int i = 0; i < s->length; i++) {
            if (s->x[i] == f->x && s->y[i] == f->y) {
                valid = 0; break;
            }
        }
        for (int i = 0; i < obs_count; i++) {
            if (obs_x[i] == f->x && obs_y[i] == f->y) {
                valid = 0; break;
            }
        }
    }
    f->type = type;
    f->spawn_time = get_tick_ms();
}

void level2_run(void) {
    int restart = 1;
    
    while (restart) {
        Snake snake;
        Food foods[MAX_FOODS];
        int food_count = 2; // 初始 1普通, 1毒药
        int obs_x[MAX_OBS];
        int obs_y[MAX_OBS];
        int obs_count = 5;  // 5个固定障碍物
        int game_over = 0;
        
        hide_cursor();
        snake_init(&snake, WIDTH / 2, HEIGHT / 2);
        init_obstacles(obs_x, obs_y, obs_count, &snake);
        
        place_food_safe_level2(&foods[0], &snake, FOOD_NORMAL, obs_x, obs_y, obs_count);
        place_food_safe_level2(&foods[1], &snake, FOOD_POISON, obs_x, obs_y, obs_count);
        
        unsigned long last_move_time = get_tick_ms();
        unsigned long base_move_interval = 150;
        unsigned long current_move_interval = base_move_interval;
        int buff_steps_left = 0;
        
        while (snake.alive && !game_over) {
            unsigned long current_time = get_tick_ms();
            
            process_input(&snake, &game_over);
            
            if (current_time - last_move_time >= current_move_interval) {
                snake_move(&snake);
                last_move_time = current_time;
                
                // 碰撞检测（墙壁、自身）
                if (check_wall_collision(&snake) || check_self_collision(&snake)) {
                    snake.alive = 0; break;
                }
                // 碰撞检测（障碍物）
                for (int i = 0; i < obs_count; i++) {
                    if (snake.x[0] == obs_x[i] && snake.y[0] == obs_y[i]) {
                        snake.alive = 0; break;
                    }
                }
                if (!snake.alive) break;
                
                // buff 倒计时
                if (buff_steps_left > 0) {
                    buff_steps_left--;
                    if (buff_steps_left == 0) {
                        current_move_interval = base_move_interval; // 恢复正常速度
                    }
                }
                
                // 吃食物检测
                for (int i = 0; i < food_count; i++) {
                    if (snake.x[0] == foods[i].x && snake.y[0] == foods[i].y) {
                        if (foods[i].type == FOOD_NORMAL) {
                            snake.score++;
                            snake_grow(&snake);
                            
                            // 吃到普通食物时，有几率刷出道具
                            if (food_count < MAX_FOODS && (rand() % 100 < 30)) {
                                int buff_type = (rand() % 2 == 0) ? FOOD_SPEED_UP : FOOD_SPEED_DOWN;
                                place_food_safe_level2(&foods[food_count], &snake, buff_type, obs_x, obs_y, obs_count);
                                food_count++;
                            }
                        } else if (foods[i].type == FOOD_POISON) {
                            snake.score--;
                            snake.length--;
                            if (snake.length < 1) { // 毒死/饿死
                                snake.alive = 0;
                            }
                        } else if (foods[i].type == FOOD_SPEED_UP) {
                            current_move_interval = base_move_interval / 2;
                            buff_steps_left = 50;
                            // 道具吃完后移出数组（与最后一个元素交换）
                            foods[i] = foods[food_count - 1];
                            food_count--;
                            continue;
                        } else if (foods[i].type == FOOD_SPEED_DOWN) {
                            current_move_interval = base_move_interval * 2;
                            buff_steps_left = 50;
                            foods[i] = foods[food_count - 1];
                            food_count--;
                            continue;
                        }
                        
                        // 普通或毒药被吃掉后，重新生成同类型
                        if (snake.alive && (foods[i].type == FOOD_NORMAL || foods[i].type == FOOD_POISON)) {
                            place_food_safe_level2(&foods[i], &snake, foods[i].type, obs_x, obs_y, obs_count);
                        }
                    }
                }
                
                if (snake.alive) {
                    draw_board_level2(&snake, foods, food_count, obs_x, obs_y, obs_count);
                }
            }
            
            Sleep(10);
        }
        
        // 游戏结束
        set_cursor_pos(0, HEIGHT + 2);
        set_color(15);
        prompt_and_update_leaderboard(snake.score);
        
        printf("\nDo you want to restart Level 2? (y/n): ");
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
