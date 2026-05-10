/*
 * level_common.c - 蛇与食物的通用操作实现（游戏逻辑核心层）
 * 功能：实现蛇的初始化、移动（头增尾删）、增长、碰撞检测
 *       以及食物安全放置等核心算法，不涉及界面和输入处理。
 *       所有关卡（level1/level2/level3）均复用本模块提供的函数。
 */
#include "../include/level_common.h"
#include <stdlib.h>

/*
 * snake_init - 初始化蛇的状态
 * 功能：将蛇放置在地图中央，初始长度为3节，方向向右，
 *       蛇身从蛇头位置向左依次排列。
 * 防呆设计：空指针检查，避免野指针导致崩溃。
 */
void snake_init(Snake *s, int startX, int startY)
{
    if (!s)
        return;               // 防呆：若传入空指针则直接返回，避免崩溃
    s->length = 3;            // 初始长度设为3节
    s->direction = DIR_RIGHT; // 初始移动方向设为向右
    s->score = 0;             // 初始得分为0
    s->alive = 1;             // 标记为存活

    // 初始化蛇身：蛇头在 (startX, startY)，身体向左延伸
    for (int i = 0; i < s->length; i++)
    {
        s->x[i] = startX - i; // 第i节的x坐标：头为 startX，下一节为 startX-1，以此类推
        s->y[i] = startY;     // 所有节的y坐标都与头同一行 startY
    }
}

/*
 * snake_move - 蛇的移动（头增尾删法）
 * 原理：从尾到头遍历蛇身数组，每一节继承前一节的坐标，
 *       然后根据当前方向更新蛇头坐标。
 *       这种方式等效于"尾部删除一节，头部新增一节"。
 * 防呆设计：空指针检查 + 死亡状态检查。
 */
void snake_move(Snake *s)
{
    if (!s || !s->alive)
        return; // 防呆：空指针和死亡状态检查

    // 蛇身跟随：从尾到头，每节继承前一节的坐标
    for (int i = s->length - 1; i > 0; i--)
    {
        s->x[i] = s->x[i - 1];
        s->y[i] = s->y[i - 1];
    }

    // 根据方向更新蛇头坐标
    switch (s->direction)
    {
    case DIR_UP:
        s->y[0]--;
        break;
    case DIR_DOWN:
        s->y[0]++;
        break;
    case DIR_LEFT:
        s->x[0]--;
        break;
    case DIR_RIGHT:
        s->x[0]++;
        break;
    }
}

/*
 * snake_grow - 蛇吃到食物后增长一节
 * 原理：将 length 加1，新节点坐标暂设为当前尾节点坐标。
 *       下一帧移动时，尾节点会自动跟随前一节移动，实现自然衔接。
 * 防呆设计：长度不超过 MAX_SNAKE，防止数组越界。
 */
void snake_grow(Snake *s)
{
    if (!s || !s->alive)
        return;
    if (s->length < MAX_SNAKE)
    {
        s->x[s->length] = s->x[s->length - 1];
        s->y[s->length] = s->y[s->length - 1];
        s->length++;
    }
}

/*
 * check_self_collision - 检测蛇是否撞到自身
 * 原理：遍历蛇身第2节到最后一节，判断是否有节点与蛇头坐标重合。
 * 返回值：1=发生碰撞, 0=未碰撞
 */
int check_self_collision(const Snake *s)
{
    if (!s)
        return 0;
    for (int i = 1; i < s->length; i++)
    {
        if (s->x[0] == s->x[i] && s->y[0] == s->y[i])
        {
            return 1;
        }
    }
    return 0;
}

/*
 * check_wall_collision - 检测蛇是否撞到边界墙壁
 * 原理：判断蛇头坐标是否位于边框上或边框外。
 *       边框占据第0行/列和第(WIDTH-1)/(HEIGHT-1)行/列，
 *       蛇的合法活动范围为 1 <= x <= WIDTH-2, 1 <= y <= HEIGHT-2。
 * 返回值：1=撞墙, 0=安全
 */
int check_wall_collision(const Snake *s)
{
    if (!s)
        return 0;
    if (s->x[0] <= 0 || s->x[0] >= WIDTH - 1 ||
        s->y[0] <= 0 || s->y[0] >= HEIGHT - 1)
    {
        return 1;
    }
    return 0;
}

/*
 * place_food_safe - 在安全位置随机放置食物
 * 原理：循环生成随机坐标，逐一检查是否与蛇身任何节点重合。
 *       若重合则重新生成，直到找到一个完全空闲的位置。
 * 防呆设计：限制最大尝试次数，防止蛇占满地图时无限循环。
 *       当蛇占满地图时返回0（可作为"游戏胜利"判定依据）。
 * 返回值：1=放置成功, 0=地图已满无法放置
 */
int place_food_safe(Food *f, const Snake *s, int type)
{
    if (!f || !s)
        return 0;

    // 计算可用空间：若蛇身已占满整个活动区域，直接返回失败
    int max_cells = (WIDTH - 2) * (HEIGHT - 2);
    if (s->length >= max_cells)
        return 0;

    int attempts = 0;
    int valid = 0;
    while (!valid && attempts < 10000)
    {
        // 在有效活动区域内生成坐标（排除边框）
        f->x = 1 + rand() % (WIDTH - 2);
        f->y = 1 + rand() % (HEIGHT - 2);
        valid = 1;
        attempts++;

        // 确保不在蛇身上
        for (int i = 0; i < s->length; i++)
        {
            if (s->x[i] == f->x && s->y[i] == f->y)
            {
                valid = 0;
                break;
            }
        }
    }
    if (!valid)
        return 0; // 尝试次数耗尽，可能地图接近满
    f->type = type;
    f->spawn_time = get_tick_ms();
    return 1;
}
