#ifndef GAME_UI_H
#define GAME_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "level_common.h"

/* 界面渲染与输入处理声明 */
void draw_board(const Snake* s, const Food* f);
void process_input(Snake* s, int* game_over);

/* 专为关卡2设计的渲染函数，支持多个食物和障碍物 */
void draw_board_level2(const Snake* s, const Food foods[], int food_count, const int obs_x[], const int obs_y[], int obs_count);

#ifdef __cplusplus
}
#endif

#endif /* GAME_UI_H */
