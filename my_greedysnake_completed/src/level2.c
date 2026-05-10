/*
 * level2.c - 第二关（极限生存模式）游戏主流程
 * 功能：在经典模式基础上新增以下进阶机制：
 *       1. 固定障碍物（深红色 M），撞击即死
 *       2. 真假食物并存（绿色普通 + 红色毒药）
 *       3. 加速道具（青色 A，移动间隔减半）
 *       4. 减速道具（紫色 V，移动间隔翻倍）
 *       道具效果持续50步后自动恢复正常速度。
 */
#define _CRT_SECURE_NO_WARNINGS
#include "../include/level2.h"
#include "../include/level_common.h"
#include "../include/game_shared.h"
#include "../include/game_ui.h"
#include "../include/leaderboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#define MAX_FOODS 4   // 同屏最大食物数（普通+毒药+可能的道具）
#define MAX_OBS 10    // 最大障碍物数量

// ==========================================================
// 核心算法1：防卡死障碍物生成 (Anti-deadlock Obstacle Generation)
// 在地图内随机生成障碍物，确保：
//   1. 坐标在有效边界内（距边框至少2格缓冲）
//   2. 绝不与蛇的初始身体重合（防止一开局就死）
//   3. 障碍物之间绝不重合
// ==========================================================
void init_obstacles(int obs_x[], int obs_y[], int obs_count, const Snake* s) {
    for (int i = 0; i < obs_count; i++) {
        int valid = 0;
        int attempts = 0;
        while (!valid && attempts < 10000) {
            obs_x[i] = 2 + rand() % (WIDTH - 4);
            obs_y[i] = 2 + rand() % (HEIGHT - 4);
            valid = 1;
            attempts++;
            
            // 检查与蛇身的碰撞
            for (int j = 0; j < s->length; j++) {
                if (s->x[j] == obs_x[i] && s->y[j] == obs_y[i]) {
                    valid = 0;
                    break;
                }
            }
            // 检查与已生成障碍物的碰撞
            if (valid) {
                for (int j = 0; j < i; j++) {
                    if (obs_x[j] == obs_x[i] && obs_y[j] == obs_y[i]) {
                        valid = 0;
                        break;
                    }
                }
            }
        }
        if (!valid) {
            obs_x[i] = 0; // Put at border wall if failed to find a spot
            obs_y[i] = 0;
        }
    }
}

// ==========================================================
// 核心算法2：安全食物放置 (Safe Food Spawning)
// 随机生成食物坐标，确保不与蛇身、障碍物、已有食物重合。
// ==========================================================
int place_food_safe_level2(Food* f, const Snake* s, int type,
                            const int obs_x[], const int obs_y[], int obs_count,
                            const Food other_foods[], int other_count) {
    /* Bug修复：原版无尝试次数上限，地图拥挤时 while(!valid) 死循环。
       加入 attempts 计数器，提升到 10000 次防止误判，超过则直接返回。 */
    int valid = 0;
    int attempts = 0;
    while (!valid && attempts < 10000) {
        f->x = 1 + rand() % (WIDTH - 2);
        f->y = 1 + rand() % (HEIGHT - 2);
        valid = 1;
        attempts++;

        /* 检查与蛇身碰撞 */
        for (int i = 0; i < s->length; i++) {
            if (s->x[i] == f->x && s->y[i] == f->y) {
                valid = 0; break;
            }
        }
        /* 检查与障碍物碰撞 */
        if (valid) {
            for (int i = 0; i < obs_count; i++) {
                if (obs_x[i] == f->x && obs_y[i] == f->y) {
                    valid = 0; break;
                }
            }
        }
        /* 检查与其他已有食物碰撞（防止两个食物重叠） */
        if (valid && other_foods != NULL) {
            for (int i = 0; i < other_count; i++) {
                if (&other_foods[i] == (const Food*)f) continue; /* 跳过自身，防止死循环 */
                if (other_foods[i].x == f->x && other_foods[i].y == f->y) {
                    valid = 0; break;
                }
            }
        }
    }
    if (!valid) return 0;
    f->type = type;
    f->spawn_time = get_tick_ms();
    return 1;
}

void level2_run(void) {
    int restart = 1;
    
    while (restart) {
        Snake snake;
        Food foods[MAX_FOODS];
        int food_count = 2;     // 初始：1个普通 + 1个毒药
        int obs_x[MAX_OBS];
        int obs_y[MAX_OBS];
        int obs_count = 5;      // 5个固定障碍物
        int game_over = 0;
        
        hide_cursor();
        clear_screen();
        
        // 关卡说明界面
        set_color(11);
        printf("\n  ===== LEVEL 2: ADVANCED MODE =====\n\n");
        set_color(15);
        printf("  Controls: Arrow Keys / WASD\n");
        printf("  New Mechanics:\n");
        set_color(4);  printf("    M "); set_color(15); printf("- Fixed obstacles (instant death)\n");
        set_color(10); printf("    $ "); set_color(15); printf("- Normal food (+1 score, +1 length)\n");
        set_color(12); printf("    X "); set_color(15); printf("- Poison food (-1 score, -1 length)\n");
        set_color(11); printf("    A "); set_color(15); printf("- Speed UP item (2x speed, 50 steps)\n");
        set_color(13); printf("    V "); set_color(15); printf("- Speed DOWN item (0.5x speed, 50 steps)\n\n");
        set_color(6);
        printf("  * Speed increases every 5 foods eaten!\n\n");
        set_color(14);
        printf("  Press any key to start...\n");
        set_color(7);
        _getch();
        clear_screen();
        snake_init(&snake, WIDTH / 2, HEIGHT / 2);
        init_obstacles(obs_x, obs_y, obs_count, &snake);
        
        // 生成初始食物（互相不重叠）
        place_food_safe_level2(&foods[0], &snake, FOOD_NORMAL, obs_x, obs_y, obs_count, NULL, 0);
        place_food_safe_level2(&foods[1], &snake, FOOD_POISON, obs_x, obs_y, obs_count, foods, 1);
        
        // 关卡差异化：Level 2 基础速度比 Level 1 快（130ms vs 150ms）
        unsigned long last_move_time = get_tick_ms();
        unsigned long base_move_interval = 130;
        unsigned long current_move_interval = base_move_interval;
        
        // 关卡差异化：Level 2 初始蛇身长度为 4（Level 1 为 3）
        snake_grow(&snake);
        snake.score = 0;  // 增长不算分
        int buff_steps_left = 0;
        int normal_foods_eaten = 0;  // 记录已吃普通食物数，每5个加速一次
        
        // 绘制初始画面
        draw_board_level2(&snake, foods, food_count, obs_x, obs_y, obs_count);
        
        // ===== 游戏主循环 =====
        while (snake.alive && !game_over) {
            unsigned long current_time = get_tick_ms();
            
            process_input(&snake, &game_over);
            
            if (current_time - last_move_time >= current_move_interval) {
                snake_move(&snake);
                last_move_time = current_time;
                
                // 碰撞检测：墙壁 + 自身
                if (check_wall_collision(&snake) || check_self_collision(&snake)) {
                    snake.alive = 0; break;
                }
                // 碰撞检测：障碍物
                for (int i = 0; i < obs_count; i++) {
                    if (snake.x[0] == obs_x[i] && snake.y[0] == obs_y[i]) {
                        snake.alive = 0; break;
                    }
                }
                if (!snake.alive) break;
                
                // 速度道具倒计时
                if (buff_steps_left > 0) {
                    buff_steps_left--;
                    if (buff_steps_left == 0) {
                        current_move_interval = base_move_interval;
                    }
                }
                
                // 吃食物检测（每帧最多吃一个，用 break 确保安全）
                for (int i = 0; i < food_count; i++) {
                    if (snake.x[0] != foods[i].x || snake.y[0] != foods[i].y) continue;
                    
                    if (foods[i].type == FOOD_NORMAL) {
                        snake.score++;
                        snake_grow(&snake);
                        normal_foods_eaten++;
                        
                        // 渐进加速：每吃 5 个普通食物，基础速度减少 10ms（最快 60ms）
                        if (normal_foods_eaten % 5 == 0 && base_move_interval > 60) {
                            base_move_interval -= 10;
                            // 如果没有道具的 buff 在生效，立即应用新速度
                            if (buff_steps_left == 0) {
                                current_move_interval = base_move_interval;
                            }
                        }
                        // 30% 概率刷出随机道具
                        if (food_count < MAX_FOODS && (rand() % 100 < 30)) {
                            int buff_type = (rand() % 2 == 0) ? FOOD_SPEED_UP : FOOD_SPEED_DOWN;
                            if (place_food_safe_level2(&foods[food_count], &snake, buff_type,
                                                   obs_x, obs_y, obs_count, foods, food_count)) {
                                food_count++;
                            }
                        }
                        // 重新生成普通食物
                        if (!place_food_safe_level2(&foods[i], &snake, FOOD_NORMAL,
                                               obs_x, obs_y, obs_count, foods, food_count)) {
                            snake.alive = 0; // 地图已满
                            break;
                        }
                        
                    } else if (foods[i].type == FOOD_POISON) {
                        snake.score--;
                        // 清除即将被截掉的尾节点的显示残影
                        set_cursor_pos(snake.x[snake.length - 1], snake.y[snake.length - 1]);
                        set_color(7);
                        printf(" ");
                        snake.length--;
                        if (snake.length < 1) {
                            snake.alive = 0;
                        }
                        // 重新生成毒食物
                        if (snake.alive) {
                            if (!place_food_safe_level2(&foods[i], &snake, FOOD_POISON,
                                                   obs_x, obs_y, obs_count, foods, food_count)) {
                                snake.alive = 0; // 地图满了
                                break;
                            }
                        }
                        
                    } else if (foods[i].type == FOOD_SPEED_UP) {
                        current_move_interval = base_move_interval / 2;
                        buff_steps_left = 50;
                        // 道具吃完后从数组移除（与末尾元素交换）
                        foods[i] = foods[food_count - 1];
                        food_count--;
                        
                    } else if (foods[i].type == FOOD_SPEED_DOWN) {
                        current_move_interval = base_move_interval * 2;
                        buff_steps_left = 50;
                        foods[i] = foods[food_count - 1];
                        food_count--;
                    }
                    break;  // 每帧最多处理一个食物
                }
                
                if (snake.alive) {
                    draw_board_level2(&snake, foods, food_count, obs_x, obs_y, obs_count);
                    // 显示速度状态提示（让老师和玩家直观看到道具生效）
                    set_cursor_pos(0, HEIGHT + 1);
                    if (buff_steps_left > 0) {
                        if (current_move_interval < base_move_interval) {
                            set_color(11);
                            printf(">> SPEED UP! %d steps left <<          ", buff_steps_left);
                        } else {
                            set_color(13);
                            printf(">> SLOW DOWN! %d steps left <<         ", buff_steps_left);
                        }
                    } else {
                        set_color(14);
                        printf("Speed: %lums (eat 5 to accelerate)     ", base_move_interval);
                    }
                }
            }
            
            Sleep(10);
        }
        
        // ===== 游戏结束处理 =====
        set_cursor_pos(0, HEIGHT + 3);
        set_color(15);
        if (game_over) {
            printf("\n==== GAME EXITED ====\n");
        } else {
            prompt_and_update_leaderboard(snake.score, 2);
        }
        
        printf("\nRestart Level 2? (y/n): ");
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
