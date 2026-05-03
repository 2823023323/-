#ifndef LEVEL_COMMON_H
#define LEVEL_COMMON_H

#include <stdint.h>
#include "game_shared.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX_SNAKE
#define MAX_SNAKE ((WIDTH - 2) * (HEIGHT - 2))
#endif

/* 蛇移动方向宏定义 */
#define DIR_UP 1
#define DIR_DOWN 2
#define DIR_LEFT 3
#define DIR_RIGHT 4

/* 蛇结构体 */
typedef struct {
    int x[MAX_SNAKE];   // 蛇身各节x坐标(蛇头索引为0)
    int y[MAX_SNAKE];   // 蛇身各节y坐标
    int length;         // 当前长度
    int direction;      // 移动方向(UP/DOWN/LEFT/RIGHT)
    int score;          // 当前得分
    int alive;          // 存活状态(1存活/ 0死亡)
} Snake;

/* 食物类型宏定义 */
#define FOOD_NORMAL 0
#define FOOD_POISON 1
#define FOOD_SPEED_UP 2
#define FOOD_SPEED_DOWN 3

/* 食物结构体 */
typedef struct {
    int x, y;                   // 食物坐标
    int type;                   // 食物类型(普通/特殊,预留扩展)
    unsigned long spawn_time;   // 生成时间戳(用于限时食物等扩展)
} Food;

/* 核心操作函数原型 */
void snake_init(Snake* s, int startX, int startY);
void snake_move(Snake* s);
void snake_grow(Snake* s);
int check_self_collision(const Snake* s);
int check_wall_collision(const Snake* s);
void place_food_safe(Food* f, const Snake* s, int type);

#ifdef __cplusplus
}
#endif

#endif /* LEVEL_COMMON_H */
