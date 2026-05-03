#include "../include/game_ui.h"
#include <stdio.h>
#include <conio.h>

void draw_board(const Snake* s, const Food* f) {
    clear_screen();
    
    // 绘制地图
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            set_cursor_pos(x, y);
            
            // 绘制边界
            if (y == 0 || y == HEIGHT - 1 || x == 0 || x == WIDTH - 1) {
                set_color(15); // 白色
                printf("#");
                continue;
            }
            
            // 绘制食物
            if (f && x == f->x && y == f->y) {
                if (f->type == FOOD_NORMAL) {
                    set_color(10); // 亮绿色
                    printf("$");
                } else if (f->type == FOOD_POISON) {
                    set_color(12); // 红色
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
                        set_color(14); // 蛇头黄色
                        printf("@");
                    } else {
                        set_color(2);  // 蛇身深绿色
                        printf("*");
                    }
                    break;
                }
            }
            
            if (!is_snake) {
                printf(" ");
            }
        }
    }
    
    // 绘制分数板
    set_cursor_pos(0, HEIGHT);
    set_color(15);
    printf("Score: %d | Length: %d\n", s->score, s->length);
}

void process_input(Snake* s, int* game_over) {
    if (_kbhit()) {
        char ch = _getch();
        if (ch == -32 || ch == 0 || ch == 224) { // 方向键扩展码
            ch = _getch();
            switch (ch) {
                case 72: // UP
                    if (s->direction != DIR_DOWN) s->direction = DIR_UP;
                    break;
                case 80: // DOWN
                    if (s->direction != DIR_UP) s->direction = DIR_DOWN;
                    break;
                case 75: // LEFT
                    if (s->direction != DIR_RIGHT) s->direction = DIR_LEFT;
                    break;
                case 77: // RIGHT
                    if (s->direction != DIR_LEFT) s->direction = DIR_RIGHT;
                    break;
            }
        } else {
            // 支持 WASD
            if ((ch == 'w' || ch == 'W') && s->direction != DIR_DOWN) s->direction = DIR_UP;
            else if ((ch == 's' || ch == 'S') && s->direction != DIR_UP) s->direction = DIR_DOWN;
            else if ((ch == 'a' || ch == 'A') && s->direction != DIR_RIGHT) s->direction = DIR_LEFT;
            else if ((ch == 'd' || ch == 'D') && s->direction != DIR_LEFT) s->direction = DIR_RIGHT;
            else if (ch == 27) { // ESC 键
                *game_over = 1;
                s->alive = 0;
            }
        }
    }
}
