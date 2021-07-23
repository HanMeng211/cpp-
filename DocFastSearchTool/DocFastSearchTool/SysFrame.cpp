#include"SysFrame.h"

#define WIDTH 120
#define HEIGHT 30

//设置当前光标的位置
void SetCurPos(int x, int y)
{
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos = { x, y };
	SetConsoleCursorPosition(handle, pos);
}
//隐藏光标
void HideCursor()
{
	CONSOLE_CURSOR_INFO cursor_info = { 100, 0 };
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorInfo(handle, &cursor_info);
}
//画列
void DrawCol(int x, int y)
{
	for (int i = 0; i < HEIGHT; ++i)
	{
		SetCurPos(x, y + i);
		printf("||");
	}
}
//画行
void DrawRow(int x, int y)
{
	for (int i = 0; i < WIDTH - 4; ++i)
	{
		SetCurPos(x + i, y);
		printf("=");
	}
}

//画框架
void DrawFrame(char *title)
{
	char buffer[MAX_TITLE_SIZE + 6 + 1] = "title "; //6:title%20 1:\0
	strcat(buffer, title);
	system(buffer); //设置系统标题

	char mode[128] = { 0 };
	sprintf(mode, "mode con cols=%d lines=%d", WIDTH, HEIGHT);
	system(mode); //设置控制台的长度和宽度
	system("color 0F");//设置颜色
	DrawCol(0, 0);
	DrawCol(WIDTH - 2, 0);
	DrawRow(2, 0);
	DrawRow(2, 2);
	DrawRow(2, 4);
	DrawRow(2, HEIGHT - 4);
	DrawRow(2, HEIGHT - 2);
}

void SystemEnd()
{
	SetCurPos(0, HEIGHT - 1);
}
extern char *title;

//画菜单
void DrawMenu()
{
	//根据自己的需求进行实现
	//标题的设置
	SetCurPos((WIDTH - 4 - strlen(title)) / 2, 1);
	printf("%s", title);
	//名称 路径
	SetCurPos(2, 3);
	printf("%-30s %-85s", "名称", "路径");
	//退出设置
	SetCurPos((WIDTH - 4 - strlen("exit 退出系统 .")) / 2, HEIGHT - 3);
	printf("%s", "exit 退出系统 .");
	DrawRow(2, HEIGHT - 6);
	//SetCurPos((WIDTH-4-strlen("请输入:>"))/2, 15);
	SetCurPos(2, HEIGHT - 5);
	printf("%s", "请输入:>");
}