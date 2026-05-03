#ifndef GAME_SHARED_H
#define GAME_SHARED_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>
#include <stdint.h>

/* 默认宽和高 */
#ifndef WIDTH
#define WIDTH 40
#endif

#ifndef HEIGHT
#define HEIGHT 20
#endif

/* 控制台系统API封装 */
void set_cursor_pos(int x, int y);
void hide_cursor(void);
void clear_screen(void);
unsigned long get_tick_ms(void);

/* 自定义：设置控制台输出颜色 */
void set_color(int color);

#ifdef __cplusplus
}
#endif

#endif /* GAME_SHARED_H */
