/*
 * level3.c - 第三关（双人对战模式）游戏主流程
 * 功能：两条蛇同屏竞技，各自由不同按键控制。
 *       P1（青色蛇）使用 WASD 控制
 *       P2（黄色蛇）使用 方向键 控制
 * 规则：先撞墙/撞自己/撞对方身体者判负。
 *       两蛇头正面对撞判定为平局（同归于尽）。
 *       两蛇抢夺同一个食物，吃到者得分+1、长度+1。
 */
#define _CRT_SECURE_NO_WARNINGS
#include "../include/level3.h"
#include "../include/level_common.h"
#include "../include/game_shared.h"
#include "../include/game_ui.h"
#include "../include/leaderboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

/*
 * place_food_safe_level3 - 在避开两条蛇的位置生成食物
 */
int place_food_safe_level3(Food* f, const Snake* s1, const Snake* s2) {
    /* Bug修复：原版无逃出机制，两蛇填满地图时死循环。
       加入地图满判断和最大尝试次数保护。 */
    int max_cells = (WIDTH - 2) * (HEIGHT - 2);
    if (s1->length + s2->length >= max_cells) return 0; /* 地图已满，无法放置 */

    int valid = 0;
    int attempts = 0;
    while (!valid && attempts < 2000) {
        f->x = 1 + rand() % (WIDTH - 2);
        f->y = 1 + rand() % (HEIGHT - 2);
        valid = 1;
        attempts++;

        for (int i = 0; i < s1->length; i++) {
            if (s1->x[i] == f->x && s1->y[i] == f->y) { valid = 0; break; }
        }
        if (!valid) continue;

        for (int i = 0; i < s2->length; i++) {
            if (s2->x[i] == f->x && s2->y[i] == f->y) { valid = 0; break; }
        }
    }
    if (!valid) return 0;
    f->type = FOOD_NORMAL;
    f->spawn_time = get_tick_ms();
    return 1;
}

/*
 * check_collision_with_other - 检测一条蛇的头是否撞到另一条蛇的【身体】
 * 注意：从 i=1 开始遍历，跳过对方蛇头（index 0）。
 *       头对头碰撞由主循环中单独的 head-on check 判定为平局。
 *       头撞身体 = 判负，头撞头 = 平局，两条逻辑完全分离。
 * 返回值：1=撞到对方身体, 0=安全
 */
int check_collision_with_other(const Snake* s, const Snake* other) {
    if (!s || !other) return 0;
    for (int i = 1; i < other->length; i++) {
        if (s->x[0] == other->x[i] && s->y[0] == other->y[i]) {
            return 1;
        }
    }
    return 0;
}

void level3_run(void) {
    int restart = 1;
    
    while (restart) {
        Snake s1, s2;
        Food food;
        int game_over = 0;
        int winner = 0;  // 0=平局, 1=P1胜, 2=P2胜
        
        hide_cursor();
        clear_screen();
        
        // 关卡说明界面
        set_color(11);
        printf("\n  ===== LEVEL 3: DOUBLE PLAYER =====\n\n");
        set_color(15);
        printf("  Two snakes compete on the same screen!\n\n");
        set_color(11); printf("  Player 1 (Blue):   "); set_color(15); printf("W/A/S/D keys\n");
        set_color(14); printf("  Player 2 (Yellow): "); set_color(15); printf("Arrow keys\n\n");
        printf("  Rules:\n");
        printf("    - Hit wall/self/opponent's body = LOSE\n");
        printf("    - Head-on collision = DRAW\n");
        printf("    - Winner saves score to leaderboard\n\n");
        set_color(14);
        printf("  Press any key to start...\n");
        set_color(7);
        _getch();
        clear_screen();
        
        // P1 在左侧出生，初始方向向右
        // snake_init 默认身体向左延伸，正好适配向右移动
        snake_init(&s1, WIDTH / 3, HEIGHT / 2);
        s1.direction = DIR_RIGHT;
        
        // P2 在右侧出生，初始方向向左
        // Bug修复：snake_init 默认身体向左延伸（x[0], x[0]-1, x[0]-2），
        //   若方向设为 DIR_LEFT，第一步蛇头左移后会与 x[2] 重合，触发自撞。
        //   解决方案：手动初始化P2，让身体向右延伸（x[0], x[0]+1, x[0]+2），
        //   这样向左移动时蛇头远离身体，不会自撞。
        snake_init(&s2, WIDTH * 2 / 3, HEIGHT / 2);
        s2.direction = DIR_LEFT;
        // 覆盖身体坐标：让身体向右延伸（与移动方向相反）
        for (int i = 0; i < s2.length; i++) {
            s2.x[i] = (WIDTH * 2 / 3) + i;
            s2.y[i] = HEIGHT / 2;
        }
        
        place_food_safe_level3(&food, &s1, &s2);
        
        unsigned long last_move_time = get_tick_ms();
        unsigned long move_interval = 120;  // 对战节奏稍快
        
        // 绘制初始画面
        draw_board_level3(&s1, &s2, &food);
        
        // ===== 游戏主循环 =====
        while (s1.alive && s2.alive && !game_over) {
            unsigned long current_time = get_tick_ms();
            
            process_input_level3(&s1, &s2, &game_over);
            
            if (current_time - last_move_time >= move_interval) {
                snake_move(&s1);
                snake_move(&s2);
                last_move_time = current_time;
                
                // --- 碰撞检测 ---
                int p1_crash = check_wall_collision(&s1) ||
                               check_self_collision(&s1) ||
                               check_collision_with_other(&s1, &s2);
                int p2_crash = check_wall_collision(&s2) ||
                               check_self_collision(&s2) ||
                               check_collision_with_other(&s2, &s1);
                
                // 特殊情况：两蛇头正面对撞
                if (s1.x[0] == s2.x[0] && s1.y[0] == s2.y[0]) {
                    p1_crash = 1;
                    p2_crash = 1;
                }
                
                if (p1_crash && p2_crash) {
                    s1.alive = 0; s2.alive = 0; winner = 0; break;
                } else if (p1_crash) {
                    s1.alive = 0; winner = 2; break;
                } else if (p2_crash) {
                    s2.alive = 0; winner = 1; break;
                }
                
                // --- 吃食物检测 ---
                int food_eaten = 0;
                if (s1.x[0] == food.x && s1.y[0] == food.y) {
                    s1.score++; snake_grow(&s1); food_eaten = 1;
                } else if (s2.x[0] == food.x && s2.y[0] == food.y) {
                    s2.score++; snake_grow(&s2); food_eaten = 1;
                }
                
                if (food_eaten) {
                    if (!place_food_safe_level3(&food, &s1, &s2)) {
                        s1.alive = 0;
                        s2.alive = 0;
                        winner = 0;
                        break;
                    }
                }
                
                draw_board_level3(&s1, &s2, &food);
            }
            Sleep(10);
        }
        
        // ===== 对战结算 =====
        set_cursor_pos(0, HEIGHT + 2);
        set_color(15);
        if (game_over) {
            /* ESC退出：不是真正的对战结果，跳过排行榜 */
            printf("\n==== GAME EXITED ====\n");
        } else {
            printf("\n==== BATTLE OVER ====\n");
            if (winner == 1) {
                set_color(11); printf("Player 1 (WASD/Blue) WINS!\n");
            } else if (winner == 2) {
                set_color(14); printf("Player 2 (Arrow/Yellow) WINS!\n");
            } else {
                set_color(15); printf("DRAW! (Mutual Destruction)\n");
            }
            set_color(15);
            printf("P1 Score: %d | P2 Score: %d\n\n", s1.score, s2.score);
            
            // 双方都可以记录到排行榜
            printf("-- Record scores to leaderboard --\n");
            set_color(11);
            prompt_player_and_record("Player 1 (Blue)", s1.score, 3);
            set_color(14);
            prompt_player_and_record("Player 2 (Yellow)", s2.score, 3);
            set_color(15);
            
            show_leaderboard(3);
        }
        
        printf("\nRestart Double Player? (y/n): ");
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
