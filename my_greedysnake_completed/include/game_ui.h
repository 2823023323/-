#ifndef GAME_UI_H
#define GAME_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "level_common.h"

/* 界面渲染与输入处理声明 */
void draw_board(const Snake* s, const Food* f);
void process_input(Snake* s, int* game_over);

#ifdef __cplusplus
}
#endif

#endif /* GAME_UI_H */
