#include "../include/level_common.h"
#include <stdlib.h>

void snake_init(Snake* s, int startX, int startY) {
    if (!s) return;
    s->length = 3;
    s->direction = DIR_RIGHT;
    s->score = 0;
    s->alive = 1;
    
    // 初始化身体，头在 startX, startY，身体向左延伸
    for (int i = 0; i < s->length; i++) {
        s->x[i] = startX - i;
        s->y[i] = startY;
    }
}

void snake_move(Snake* s) {
    if (!s || !s->alive) return;
    
    // 蛇身跟随前一节
    for (int i = s->length - 1; i > 0; i--) {
        s->x[i] = s->x[i - 1];
        s->y[i] = s->y[i - 1];
    }
    
    // 根据方向更新头
    switch (s->direction) {
        case DIR_UP:    s->y[0]--; break;
        case DIR_DOWN:  s->y[0]++; break;
        case DIR_LEFT:  s->x[0]--; break;
        case DIR_RIGHT: s->x[0]++; break;
    }
}

void snake_grow(Snake* s) {
    if (!s || !s->alive) return;
    if (s->length < MAX_SNAKE) {
        // 新增的节点坐标暂设为当前尾节点坐标，下一帧移动时会自动铺开
        s->x[s->length] = s->x[s->length - 1];
        s->y[s->length] = s->y[s->length - 1];
        s->length++;
    }
}

int check_self_collision(const Snake* s) {
    if (!s) return 0;
    // 从第二节开始检查是否与头重合
    for (int i = 1; i < s->length; i++) {
        if (s->x[0] == s->x[i] && s->y[0] == s->y[i]) {
            return 1; // 发生碰撞
        }
    }
    return 0;
}

int check_wall_collision(const Snake* s) {
    if (!s) return 0;
    // 假设边界为 0 到 WIDTH-1 和 0 到 HEIGHT-1
    if (s->x[0] <= 0 || s->x[0] >= WIDTH - 1 || s->y[0] <= 0 || s->y[0] >= HEIGHT - 1) {
        return 1;
    }
    return 0;
}

void place_food_safe(Food* f, const Snake* s, int type) {
    if (!f || !s) return;
    
    int valid = 0;
    while (!valid) {
        // 在边界内生成坐标 1 到 WIDTH-2, 1 到 HEIGHT-2
        f->x = 1 + rand() % (WIDTH - 2);
        f->y = 1 + rand() % (HEIGHT - 2);
        valid = 1;
        
        // 确保不在蛇身上
        for (int i = 0; i < s->length; i++) {
            if (s->x[i] == f->x && s->y[i] == f->y) {
                valid = 0;
                break;
            }
        }
    }
    f->type = type;
    f->spawn_time = get_tick_ms();
}
