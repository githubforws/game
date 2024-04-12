#include<graphics.h>
#include<string>
#include<iostream>
#include<vector>
using namespace std;
#pragma comment(lib,"MSIMG32.LIB")
const int FPS = 144;//帧率
const int NUMS = 6;//帧率
const int button_gamebegin_x = 500;//开始游戏按钮坐标
const int button_gamebegin_y = 400;
const int button_gameexit_x = 500;//退出游戏按钮坐标
const int button_gameexit_y = 500;
int player_x = 0;//玩家x坐标
int player_y = 400;//玩家y坐标
int player_speed = 2;//玩家奔跑速度
int scene_now = 0;//目前在哪一幕
bool running = true;
ExMessage msg;
IMAGE button_game_begin;
IMAGE button_game_exit;
IMAGE player;
IMAGE ui_background;//主菜单背景
IMAGE atlas_player_stand_right[NUMS];//玩家站立朝右图集
IMAGE atlas_player_stand_left[NUMS];//玩家站立朝左图集
IMAGE atlas_player_run_right[NUMS];//玩家奔跑朝右图集
IMAGE atlas_player_run_left[NUMS];//玩家奔跑朝左图集
IMAGE atlas_enemy_right[NUMS];//敌人朝右奔跑图集
IMAGE atlas_enemy_left[NUMS];//敌人朝左奔跑图集
IMAGE atlas_speak[NUMS];//对话图集
inline void putimage_alpha(int x, int y, IMAGE* img) {//能处理透明背景的渲染函数
	int w = img->getwidth();
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h,
		GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });
}
void loadall() {
	loadimage(&button_game_begin, _T("img/ui_begin.png"), 192, 75);
	loadimage(&button_game_exit, _T("img/ui_exit.png"), 192, 75);
	loadimage(&ui_background, _T("img/ui_background.png"), 1280, 720);
	loadimage(&player, _T("img/player.png"), 200, 200);
	for (int i = 0; i < NUMS; i++) {
		static TCHAR img_path[256];
		_stprintf_s(img_path, _T("img/player_left_%d.png"), i);
		loadimage(&atlas_player_stand_left[i], img_path);
		int width = atlas_player_stand_left[i].getwidth();
		int height = atlas_player_stand_left[i].getheight();
		Resize(&atlas_player_stand_right[i], width, height);
		DWORD* color_buffer_left = GetImageBuffer(&atlas_player_stand_left[i]);
		DWORD* color_buffer_right = GetImageBuffer(&atlas_player_stand_right[i]);
		for (int yy = 0; yy < height; yy++) {
			for (int xx = 0; xx < width; xx++) {
				int idx_left = yy * width + xx;
				int idx_right = yy * width + (width - xx - 1);
				color_buffer_right[idx_right] = color_buffer_left[idx_left];
			}
		}
	}
	for (int i = 0; i < NUMS; i++) {
		static TCHAR img_path[256];
		_stprintf_s(img_path, _T("img/player_run_left_%d.png"), i);
		loadimage(&atlas_player_run_left[i], img_path);
		int width = atlas_player_run_left[i].getwidth();
		int height = atlas_player_run_left[i].getheight();
		Resize(&atlas_player_run_right[i], width, height);
		DWORD* color_buffer_left = GetImageBuffer(&atlas_player_run_left[i]);
		DWORD* color_buffer_right = GetImageBuffer(&atlas_player_run_right[i]);
		for (int yy = 0; yy < height; yy++) {
			for (int xx = 0; xx < width; xx++) {
				int idx_left = yy * width + xx;
				int idx_right = yy * width + (width - xx - 1);
				color_buffer_right[idx_right] = color_buffer_left[idx_left];
			}
		}
	}
	for (int i = 0; i < NUMS; i++) {
		static TCHAR img_path[256];
		_stprintf_s(img_path, _T("img/enemy_left_%d.png"), i);
		loadimage(&atlas_enemy_left[i], img_path);
		int width = atlas_enemy_left[i].getwidth();
		int height = atlas_enemy_left[i].getheight();
		Resize(&atlas_enemy_right[i], width, height);
		DWORD* color_buffer_left = GetImageBuffer(&atlas_enemy_left[i]);
		DWORD* color_buffer_right = GetImageBuffer(&atlas_enemy_right[i]);
		for (int yy = 0; yy < height; yy++) {
			for (int xx = 0; xx < width; xx++) {
				int idx_left = yy * width + xx;
				int idx_right = yy * width + (width - xx - 1);
				color_buffer_right[idx_right] = color_buffer_left[idx_left];
			}
		}
	}

}
int main() {
	loadall();
	initgraph(1280, 720);
	BeginBatchDraw();
	while (running) {
		DWORD frame_strat_time = GetTickCount();
		if (scene_now == 0) {//主菜单场景
			static bool is_begin_down = false;
			static bool is_exit_down = false;
			while (peekmessage(&msg)) {
				if (msg.message == WM_LBUTTONDOWN) {//如果按下就标记
					if (msg.x > button_gamebegin_x-5 && msg.x < button_gamebegin_x + 205 && msg.y > button_gamebegin_y-5 && msg.y < button_gamebegin_y + 55) {
						is_begin_down = true;
					}
					if (msg.x > button_gameexit_x - 5 && msg.x < button_gameexit_x + 205 && msg.y > button_gameexit_y - 5 && msg.y < button_gameexit_y + 55) {
						is_exit_down = true;
					}
				}
				else if (msg.message == WM_LBUTTONUP) {//如果松开并且已经有标记就进入下一个场景，即按下开始游戏按钮
					if (msg.x > button_gamebegin_x-5 && msg.x < button_gamebegin_x + 205 && msg.y > button_gamebegin_y-5 && msg.y < button_gamebegin_y + 55) {
						if (is_begin_down) {
							scene_now = 1;
							break;
						}
					}
					if (msg.x > button_gameexit_x - 5 && msg.x < button_gameexit_x + 205 && msg.y > button_gameexit_y - 5 && msg.y < button_gameexit_y + 55) {
						if (is_exit_down) {
							running = false;
							break;
						}
					}
				}
			}
			cleardevice();
			putimage_alpha(0, 0, &ui_background);//画背景
			putimage_alpha(button_gamebegin_x, button_gamebegin_y, &button_game_begin);//画开始游戏按钮图像
			putimage_alpha(button_gameexit_x, button_gameexit_y, &button_game_exit);//画退出游戏按钮图像
			FlushBatchDraw();
		}
		else if (scene_now == 1) {//第一幕：实现对话，跳跃，奔跑
			static bool is_left = false;
			static bool is_right = false;
			DWORD frame_strat_time = GetTickCount();
			while (peekmessage(&msg)) {
				if (msg.message == WM_KEYUP) {
					if (msg.vkcode == VK_LEFT) {
						is_left=true;
					}
					else if (msg.vkcode == VK_RIGHT) {
						is_right = true;
					}
				}
				else if (msg.message == WM_KEYDOWN) {
					if (msg.vkcode == VK_LEFT) {
						is_left = false;
					}
					else if (msg.vkcode == VK_RIGHT) {
						is_right = false;
					}
				}
				
			}
			if (is_left) {
				player_x += player_speed;
			}
			if (is_right) {
				player_x -= player_speed;
				if (player_x < -50)player_x =-50;
			}
			cleardevice();
			putimage_alpha(player_x, player_y, &player);
			FlushBatchDraw();
		}
		else if (scene_now == 2) {//对话，得到武器：圆

		}
		else if (scene_now == 3) {

		}
		else if (scene_now == 4) {

		}
		else if (scene_now == 5) {

		}
		else if (scene_now == 6) {

		}
		else if (scene_now == 7) {

		}
		DWORD frame_end_time = GetTickCount();
		DWORD frame_delte_time = frame_strat_time - frame_end_time;
		if (frame_delte_time < 1000 / FPS) {
			Sleep(1000 / FPS - frame_delte_time);
		}
	}
	EndBatchDraw();

	return 0;
}