#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

    /* ���ⲿδ���� NAME_LEN/LEADERBOARD_FILE/LEADERBOARD_SIZE����ʹ��Ĭ��ֵ */
#ifndef NAME_LEN
#define NAME_LEN 32
#endif

#ifndef LEADERBOARD_FILE
#define LEADERBOARD_FILE "leaderboard.txt"
#endif

#ifndef LEADERBOARD_SIZE
#define LEADERBOARD_SIZE 5
#endif

    typedef struct {
        char name[NAME_LEN];
        int score;
    } LBEntry;

    /* ������� max ����¼������ʵ�ʶ�ȡ���� */
    int load_leaderboard(LBEntry arr[], int max);
    /* ���� count ����¼���ļ������ǣ� */
    void save_leaderboard(const LBEntry arr[], int count);

    /* ��һ����¼�ϲ������а��ڲ������򲢽ضϵ� LEADERBOARD_SIZE�� */
    void update_leaderboard(const char* name, int score);

    /* �ڿ���̨��ʾ����������ֲ�������д�����а�*/
    void prompt_and_update_leaderboard(int final_score);


#ifdef __cplusplus
}
#endif

#endif /* LEADERBOARD_H */