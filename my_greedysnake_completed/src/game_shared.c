/*
 * game_shared.c - 控制台底层系统接口封装
 * 功能：封装Windows控制台API，提供光标定位、光标隐藏、清屏、
 *       系统时间获取以及控制台颜色设置等基础服务。
 *       所有其他模块均依赖此层进行控制台交互。
 */
#include "../include/game_shared.h"
#include <stdio.h>

/*
 * set_cursor_pos - 将控制台光标移动到指定坐标 (x, y)
 * 原理：通过 GetStdHandle 获取标准输出句柄，
 *       再用 SetConsoleCursorPosition 设置光标位置。
 * 参数：x - 列坐标, y - 行坐标 (均从0开始)
 */
void set_cursor_pos(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hConsole, pos);
}

/*
 * hide_cursor - 隐藏控制台光标
 * 原理：获取当前光标信息，将 bVisible 属性设为 FALSE，
 *       避免游戏运行时光标闪烁干扰视觉体验。
 */
void hide_cursor(void) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

/*
 * clear_screen - 清空控制台屏幕
 * 原理：调用系统命令 cls 实现全屏清除。
 *       注意：频繁调用会导致画面闪烁，游戏主循环中应改用
 *       set_cursor_pos(0,0) 进行光标归位覆盖绘制。
 */
void clear_screen(void) {
    system("cls");
}

/*
 * get_tick_ms - 获取系统启动后经过的毫秒数
 * 原理：调用 Windows API GetTickCount()，
 *       用于计算帧间隔、控制蛇的移动速度。
 */
unsigned long get_tick_ms(void) {
    return GetTickCount();
}

/*
 * set_color - 设置控制台输出文字的颜色
 * 原理：通过 SetConsoleTextAttribute 改变后续 printf 输出的颜色。
 * 参数：color - Windows控制台颜色代码
 *       常用值：2=深绿, 3=深青, 4=深红, 6=深黄,
 *              10=亮绿, 11=亮青, 12=亮红, 13=紫, 14=亮黄, 15=白
 */
void set_color(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}
