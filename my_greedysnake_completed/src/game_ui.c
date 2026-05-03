/*
 * game_ui.c - 界面渲染与输入处理实现
 * 功能：绘制游戏画面（边框、蛇、食物、分数面板），
 *       处理键盘输入（支持方向键和WASD双模式），
 *       并为不同关卡提供差异化的渲染函数。
 *       使用光标归位法（而非清屏）避免画面闪烁。
 */
#include "../include/game_ui.h"
#include <stdio.h>
#include <conio.h>

/*
 * draw_board - 第一关经典模式的画面绘制
 * 原理：将光标归位到 (0,0) 后逐格覆盖绘制，避免调用 clear_screen 导致闪烁。
 *       绘制优先级：边框 > 食物 > 蛇 > 空白
 */
void draw_board(const Snake* s, const Food* f) {
    set_cursor_pos(0, 0);
    
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            set_cursor_pos(x, y);
            
            // 绘制边界
            if (y == 0 || y == HEIGHT - 1 || x == 0 || x == WIDTH - 1) {
                set_color(15);
                printf("#");
                continue;
            }
            
            // 绘制食物
            if (f && x == f->x && y == f->y) {
                if (f->type == FOOD_NORMAL) {
                    set_color(10);
                    printf("$");
                } else if (f->type == FOOD_POISON) {
                    set_color(12);
                    printf("X");
                }
                continue;
            }
            
            // 绘制蛇
            int is_snake = 0;
            for (int i = 0; i < s->length; i++) {
                if (s->x[i] == x && s->y[i] == y) {
                    is_snake = 1;
                    if (i == 0) {
                        set_color(14);  // 蛇头：亮黄色
                        printf("@");
                    } else {
                        set_color(2);   // 蛇身：深绿色
                        printf("*");
                    }
                    break;
                }
            }
            
            if (!is_snake) {
                set_color(7);  // 重置为默认色再输出空格，防止残留色块
                printf(" ");
            }
        }
    }
    
    // 绘制分数面板
    set_cursor_pos(0, HEIGHT);
    set_color(15);
    printf("Score: %d | Length: %d          \n", s->score, s->length);
}

/*
 * process_input - 处理单人模式的键盘输入
 * 原理：使用 _kbhit() 无阻塞检测按键，_getch() 读取键值。
 *       方向键会返回两个字节（前导码 + 实际键值），需特殊处理。
 *       内置防反向逻辑：正在向上走时，按下不生效（防止自杀）。
 */
void process_input(Snake* s, int* game_over) {
    if (_kbhit()) {
        char ch = _getch();
        if (ch == -32 || ch == 0 || ch == (char)224) {  // 方向键前导码
            ch = _getch();
            switch (ch) {
                case 72: if (s->direction != DIR_DOWN)  s->direction = DIR_UP;    break;
                case 80: if (s->direction != DIR_UP)    s->direction = DIR_DOWN;  break;
                case 75: if (s->direction != DIR_RIGHT) s->direction = DIR_LEFT;  break;
                case 77: if (s->direction != DIR_LEFT)  s->direction = DIR_RIGHT; break;
            }
        } else {
            // 支持 WASD 字母键控制
            if      ((ch == 'w' || ch == 'W') && s->direction != DIR_DOWN)  s->direction = DIR_UP;
            else if ((ch == 's' || ch == 'S') && s->direction != DIR_UP)    s->direction = DIR_DOWN;
            else if ((ch == 'a' || ch == 'A') && s->direction != DIR_RIGHT) s->direction = DIR_LEFT;
            else if ((ch == 'd' || ch == 'D') && s->direction != DIR_LEFT)  s->direction = DIR_RIGHT;
            else if (ch == 27) {  // ESC 键退出
                *game_over = 1;
                s->alive = 0;
            }
        }
    }
}

/*
 * draw_board_level2 - 第二关进阶模式的画面绘制
 * 功能：在经典绘制基础上，增加障碍物和多种食物类型的渲染。
 *       绘制优先级：边框 > 障碍物 > 食物 > 蛇 > 空白
 */
void draw_board_level2(const Snake* s, const Food foods[], int food_count,
                       const int obs_x[], const int obs_y[], int obs_count) {
    set_cursor_pos(0, 0);
    
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            set_cursor_pos(x, y);
            
            if (y == 0 || y == HEIGHT - 1 || x == 0 || x == WIDTH - 1) {
                set_color(15);
                printf("#");
                continue;
            }
            
            // 绘制障碍物（深红色 M）
            int is_obs = 0;
            for (int i = 0; i < obs_count; i++) {
                if (obs_x[i] == x && obs_y[i] == y) {
                    set_color(4);
                    printf("M");
                    is_obs = 1;
                    break;
                }
            }
            if (is_obs) continue;
            
            // 绘制各类食物
            int is_food = 0;
            for (int i = 0; i < food_count; i++) {
                if (foods[i].x == x && foods[i].y == y) {
                    if (foods[i].type == FOOD_NORMAL) {
                        set_color(10);  // 亮绿：普通食物
                        printf("$");
                    } else if (foods[i].type == FOOD_POISON) {
                        set_color(12);  // 亮红：毒食物
                        printf("X");
                    } else if (foods[i].type == FOOD_SPEED_UP) {
                        set_color(11);  // 青色：加速道具
                        printf("A");
                    } else if (foods[i].type == FOOD_SPEED_DOWN) {
                        set_color(13);  // 紫色：减速道具
                        printf("V");
                    }
                    is_food = 1;
                    break;
                }
            }
            if (is_food) continue;
            
            // 绘制蛇
            int is_snake = 0;
            for (int i = 0; i < s->length; i++) {
                if (s->x[i] == x && s->y[i] == y) {
                    is_snake = 1;
                    if (i == 0) {
                        set_color(14);
                        printf("@");
                    } else {
                        set_color(2);
                        printf("*");
                    }
                    break;
                }
            }
            
            if (!is_snake) {
                set_color(7);
                printf(" ");
            }
        }
    }
    
    // 绘制分数面板
    set_cursor_pos(0, HEIGHT);
    set_color(15);
    printf("Score: %d | Length: %d | Level 2          \n", s->score, s->length);
}

/*
 * draw_board_level4 - 第四关限时生存模式的画面绘制
 * 功能：在经典绘制基础上，增加多个移动障碍物（亮红色 O）和倒计时显示。
 */
void draw_board_level4(const Snake* s, const Food* f, const int mob_x[], const int mob_y[], int mob_count, int time_left) {
    set_cursor_pos(0, 0);
    
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            set_cursor_pos(x, y);
            
            if (y == 0 || y == HEIGHT - 1 || x == 0 || x == WIDTH - 1) {
                set_color(15);
                printf("#");
                continue;
            }
            
            // 绘制移动障碍物（亮红色 O）
            int is_mob = 0;
            for (int i = 0; i < mob_count; i++) {
                if (x == mob_x[i] && y == mob_y[i]) {
                    set_color(12);
                    printf("O");
                    is_mob = 1;
                    break;
                }
            }
            if (is_mob) continue;
            
            // 绘制普通食物
            if (f && x == f->x && y == f->y) {
                set_color(10);
                printf("$");
                continue;
            }
            
            // 绘制蛇
            int is_snake = 0;
            for (int i = 0; i < s->length; i++) {
                if (s->x[i] == x && s->y[i] == y) {
                    is_snake = 1;
                    if (i == 0) {
                        set_color(14);
                        printf("@");
                    } else {
                        set_color(2);
                        printf("*");
                    }
                    break;
                }
            }
            
            if (!is_snake) {
                set_color(7);
                printf(" ");
            }
        }
    }
    
    // 绘制限时分数面板
    set_cursor_pos(0, HEIGHT);
    set_color(15);
    printf("Score: %d | Length: %d | Time Left: %02ds          \n", s->score, s->length, time_left);
}

/*
 * draw_board_level3 - 第三关双人对战模式的画面绘制
 * 功能：同屏渲染两条蛇（P1蓝色系 / P2黄色系）和一个食物。
 */
void draw_board_level3(const Snake* s1, const Snake* s2, const Food* f) {
    set_cursor_pos(0, 0);
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            set_cursor_pos(x, y);
            if (y == 0 || y == HEIGHT - 1 || x == 0 || x == WIDTH - 1) {
                set_color(15);
                printf("#");
                continue;
            }
            if (f && x == f->x && y == f->y) {
                set_color(10);
                printf("$");
                continue;
            }
            int is_snake = 0;
            // 绘制玩家1（青色系）
            for (int i = 0; i < s1->length; i++) {
                if (s1->x[i] == x && s1->y[i] == y) {
                    is_snake = 1;
                    if (i == 0) { set_color(11); printf("@"); }  // P1头：亮青
                    else        { set_color(3);  printf("*"); }  // P1身：深青
                    break;
                }
            }
            if (is_snake) continue;
            // 绘制玩家2（黄色系）
            for (int i = 0; i < s2->length; i++) {
                if (s2->x[i] == x && s2->y[i] == y) {
                    is_snake = 1;
                    if (i == 0) { set_color(14); printf("@"); }  // P2头：亮黄
                    else        { set_color(6);  printf("*"); }  // P2身：深黄
                    break;
                }
            }
            if (!is_snake) {
                set_color(7);
                printf(" ");
            }
        }
    }
    set_cursor_pos(0, HEIGHT);
    set_color(11); printf("P1(WASD) Score:%d   ", s1->score);
    set_color(14); printf("P2(Arrow) Score:%d          \n", s2->score);
}

/*
 * process_input_level3 - 双人对战模式的输入处理
 * 按键分配：P1 使用 WASD，P2 使用方向键 ↑↓←→
 * 同样内置防反向逻辑。
 */
void process_input_level3(Snake* s1, Snake* s2, int* game_over) {
    if (_kbhit()) {
        char ch = _getch();
        if (ch == -32 || ch == 0 || ch == (char)224) {
            ch = _getch();
            switch (ch) {
                case 72: if (s2->direction != DIR_DOWN)  s2->direction = DIR_UP;    break;
                case 80: if (s2->direction != DIR_UP)    s2->direction = DIR_DOWN;  break;
                case 75: if (s2->direction != DIR_RIGHT) s2->direction = DIR_LEFT;  break;
                case 77: if (s2->direction != DIR_LEFT)  s2->direction = DIR_RIGHT; break;
            }
        } else {
            if      ((ch == 'w' || ch == 'W') && s1->direction != DIR_DOWN)  s1->direction = DIR_UP;
            else if ((ch == 's' || ch == 'S') && s1->direction != DIR_UP)    s1->direction = DIR_DOWN;
            else if ((ch == 'a' || ch == 'A') && s1->direction != DIR_RIGHT) s1->direction = DIR_LEFT;
            else if ((ch == 'd' || ch == 'D') && s1->direction != DIR_LEFT)  s1->direction = DIR_RIGHT;
            else if (ch == 27) {
                *game_over = 1;
                s1->alive = 0;
                s2->alive = 0;
            }
        }
    }
}
