#include<graphics.h>
#include<string>
#include<iostream>
#include<vector>
#include<math.h>
using namespace std;
#pragma comment(lib,"MSIMG32.LIB")
#pragma comment(lib,"Winmm.lib")

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
int ans = 0;
const int FPS = 50;//帧率
const int NUMS = 4;
//const int frame_conversion = 7;
const int button_gamebegin_x = 550;//开始游戏按钮坐标
const int button_gamebegin_y = 400;
const int button_gameexit_x = 550;//退出游戏按钮坐标
const int button_gameexit_y = 500;
int scene_now = 0;//目前在哪一幕
int idx_play = 0;//动画帧
int last_fx = 0;//最近的一次朝向
bool running = true;
ExMessage msg;
IMAGE button_game_begin;
IMAGE button_game_exit;
IMAGE player;
IMAGE ui_background;//主菜单背景
IMAGE atlas_player_stand_right;//玩家站立朝右图集
IMAGE atlas_player_stand_left;//玩家站立朝左图集
IMAGE atlas_player_run_right[NUMS];//玩家奔跑朝右图集
IMAGE atlas_player_run_left[NUMS];//玩家奔跑朝左图集
IMAGE atlas_enemy_right[NUMS];//敌人朝右奔跑图集
IMAGE atlas_enemy_left[NUMS];//敌人朝左奔跑图集
IMAGE atlas_speak[NUMS];//对话图集
IMAGE speak;
IMAGE white_all;
TCHAR m1[3][100] = { _T("神秘声音: 你是何人，为何会来此？a"),_T("你:...a"),_T("神秘声音: 嗯？不会说话却又有实体，难道...有趣，真是有趣。或许你能帮到我们...来吧，向前走几步，你就能看到我们，看到圆了。a") };
TCHAR m2[3][100] = { _T("神秘声音: 看到了吗？如此的美。圆，这世界上最伟大而神秘，最完美的存在。可如今却被困在这里。或许你能...a"),_T("话音未落，你身后传来动静...a"),_T("神秘声音: 来的真快啊，来不及了，快靠近圆，将其唤醒，我会帮你的，来不及解释了，快!a") };
TCHAR m3[3][100] = { _T("神秘声音: 现在你算是初步得到了圆的认可了，操控圆把那些东西干掉。a"),_T("神秘声音: 不错嘛，领悟得还挺快。对了，忘记自我介绍了，我是圆的代理人之一，你可以叫我--祖冲之。a"),_T("唔...既然你能救出圆，那一定也能救出另一位代理人，我的前辈--刘徽。前进吧，一起去救出他，你会有收获的。a") };

inline void putimage_alpha(int x, int y, IMAGE* img) {//能处理透明背景的渲染函数
	int w = img->getwidth();
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h,
		GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });
}

//动画加载类
class Animation {
private:
	std::vector<IMAGE*> frame_list;	//动画帧列表
	int interval_ms=0;//动画间隔
	int timer = 0;//动画计时器
	int idx_frame = 0;//动画帧索引
public:
	Animation(LPCTSTR path, int num, int interval) {
		interval_ms = interval;
		TCHAR path_file[256];
		for (int i = 0; i <= num; i++) {
			_stprintf_s(path_file, path, i);
			IMAGE* frame = new IMAGE();
			loadimage(frame, path_file);
			frame_list.push_back(frame);//添加动画帧
		}
	}
	~Animation() {
		for (auto& frame : frame_list) {
			delete frame;//释放动画帧
		}
	}

	void play(int x, int y, int delta) {
		timer += delta;//更新计时器
		if (timer >= interval_ms) {
			idx_frame = (idx_frame + 1) % frame_list.size();//切换下一帧
			timer = 0;//重置计时器
		}
		putimage_alpha(x, y, frame_list[idx_frame]);//渲染动画帧
	}
};


//武器圆类
class CIRCLE_GOD {
public:
	CIRCLE_GOD() = default;
	~CIRCLE_GOD() = default;
	void o_draw() {//画圆
		circle(oo_x, oo_y, r);
	}
	void Do_attack() {//攻击
		if (fx_attack) {
			oo_x -= oo_speed;
		}
		else {
			oo_x += oo_speed;
		}
		oo_attack_dist++;
		if (oo_attack_dist > 15) {//如果达到攻击最大距离，就返回主角身边
			attack_form = 2;
		}
	}
	void With(int x, int y, int fx) {//跟随人物
		if (fx) {
			oo_x = x - 20;
			oo_y = y + 50;
		}
		else {
			oo_x = x + 100;
			oo_y = y + 50;
		}
	}
	void Back(int x, int y) {//返回人物身边
		int dir_x = x - oo_x;
		int dir_y = y - oo_y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir > 50) {
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			oo_x += (int)(oo_speed * normalized_x);
			oo_y += (int)(oo_speed * normalized_y);
		}
		else {
			attack_form = 0;
			oo_attack_dist = 0;
		}
	}
public:
	int oo_form = 0;//圆当前形态
	int oo_x;
	int oo_y;
	int r = 15;
	int oo_speed = 15;//速度
	int oo_attack_dist = 0;//攻击弹道
	int attack_form = 0;//攻击过程中的阶段
	bool fx_attack = false;//攻击方向
};
class HUANGFANG {
public:
	void on_draw() {
		rectangle(700, 50, 1000, 350);
		line(700, 150, 1000, 150);
		line(700, 250, 1000, 250);
		line(800, 50, 800, 350);
		line(900, 50, 900, 350);
		for (int i = 1; i <= 9; i++) {
			circle(grid_x[i], grid_y[i], circle_size[i] * 5);
		}
	}
	void message_cheak(const ExMessage& msg) {
		if (msg.message == WM_LBUTTONDOWN) {
			for (int i = 1; i <= 9; i++) {
				if (msg.x<grid_x[i] + 50 && msg.x>grid_x[i] - 50 && msg.y<grid_y[i] + 50 && msg.y>grid_y[i] - 50) {
					if (should_chang == i) {
						should_chang = 0;
					}
					else {
						should_chang = i;
					}
					break;
				}
			}
		}
		else if (msg.message == WM_MOUSEWHEEL) {
			int kk = msg.wheel / 120;
			if (should_chang) {
				circle_size[should_chang] += kk;
				circle_size[should_chang] = max(0, circle_size[should_chang]);
				circle_size[should_chang] = min(9, circle_size[should_chang]);
			}
		}
	}
	bool win_cheak() {//判断幻方是否完成
		for (int i = 1; i <= 9; i++) {
			if (circle_size[i] != grid_ans[i]) {
				return false;
			}
		}
		return true;
	}

public:
	int circle_size[10] = { 0 };
	int grid_x[10] = { 0 , 750 , 850 , 950 , 750 , 850 , 950 , 750 , 850 , 950 };
	int grid_y[10] = { 0 , 100 , 100 , 100 , 200 , 200 , 200 , 300 , 300 , 300 };
	int should_chang = 0;
	int grid_ans[10] = { 0 ,4 , 9 , 2 , 3 , 5 , 7 , 8 , 1 , 6 };
};
class PLAYER {
public:

	void Draw() {//画人物和血条
		for (int i = 1; i <= hp; i++) {
			setfillcolor(RGB(237, 28, 36));
			solidcircle(i * 30, 30, 10);
		}
		if (is_left && is_right || !is_left && !is_right) {
			if (last_fx) {
				putimage_alpha(x, y, &atlas_player_stand_left);
			}
			else {
				putimage_alpha(x, y, &atlas_player_stand_right);
			}
		}
		else {

			if (last_fx) {
				putimage_alpha(x, y, &atlas_player_run_left[idx_play]);
			}
			else {
				putimage_alpha(x, y, &atlas_player_run_right[idx_play]);
			}
		}

	}
	void Message_cheak(const ExMessage& img) {//处理消息
		if (msg.message == WM_KEYDOWN) {
			if (msg.vkcode == 0x41) {
				is_left = true;
			}
			else if (msg.vkcode == 0x44) {
				is_right = true;
			}
			else if (msg.vkcode == VK_SPACE && is_jump == 0) {
				is_jump = 31;
			}
			else if (msg.vkcode == 0x4A) {
				is_attack = true;
			}
		}
		else if (msg.message == WM_KEYUP) {
			if (msg.vkcode == 0x41) {
				is_left = false;
			}
			else if (msg.vkcode == 0x44) {
				is_right = false;
			}

		}
	}
	void Move() {//人物移动及跳跃
		if (is_left) {
			x -= speed;
			if (x < -30)x = -30;
			last_fx = 1;
		}
		if (is_right) {
			x += speed;
			last_fx = 0;
		}
		if (is_jump) {//跳跃
			y -= is_jump - 16;
			is_jump--;
		}
	}

public:
	int x;
	int y;
	bool is_left = false;
	bool is_right = false;
	bool is_attack = false;
	int is_jump = 0;
	bool first_attack = false;
	int speed = 5;//玩家奔跑速度
	int hp;
};


//敌人类
class Enemy {
private:
	const int SPEED = 2;		//敌人速度	
	const int FRAME_WIDTH = 44;//敌人宽度
	const int FRAME_HEIGHT = 50;//敌人高度
	//const int SHADOW_WIDTH = 48;//阴影

	int player_x;
	int player_y;
	IMAGE img_shadow;
	Animation* anim_left;//
	Animation* anim_right;//
	POINT position = { 0,0 };//位置
	bool facing_left = false;//朝向
	bool alived = true;//存活
public:
	Enemy() {
		loadimage(&img_shadow, _T("img/shadow_enemy.png"));//阴影
		anim_left = new Animation(_T("img/e%d.png"), 4, 1000/FPS);//左动画，4张，间隔
		anim_right = new Animation(_T("img/e%d.png"), 4, 1000/FPS);//右动画，4张，间隔


		//敌人初始位置
		enum class SpawnEdge {
			//UP = 0,
			//DOWN,
			LEFT=0,
			RIGHT,
		};
		//随机生成敌人位置
		SpawnEdge edge = (SpawnEdge)(rand() % 2);
		switch (edge) {
		/*
		case SpawnEdge::UP:
			position.x = rand() % WINDOW_WIDTH;
			position.y = -FRAME_HEIGHT;
			break;
		case SpawnEdge::DOWN:
			position.x = rand() % WINDOW_WIDTH;
			position.y = WINDOW_HEIGHT;
			break;
		*/
		case SpawnEdge::LEFT:
			position.x = -FRAME_WIDTH;
			position.y = 380+ FRAME_HEIGHT;
			break;
		case SpawnEdge::RIGHT:
			position.x = WINDOW_WIDTH;
			position.y = 380+ FRAME_HEIGHT;
			break;
		}

	}
	~Enemy() {
		delete anim_left;
		delete anim_right;
	}

	void Hurt() {
		alived = false;
	}

	bool CheckAlived() {
		return alived;
	}

	bool CheckBulletCollision(const CIRCLE_GOD& bullet) {
		//圆是武器，判断碰撞
		bool is_overlap_x = ((bullet.oo_x > position.x) && (bullet.oo_x < position.x + FRAME_WIDTH));
		bool is_overlap_y = ((bullet.oo_y > position.y - FRAME_HEIGHT) && (bullet.oo_y < position.y + FRAME_HEIGHT));
		return is_overlap_x && is_overlap_y;

	}
	bool CheckPlayerCollision(const PLAYER& player) {
		//玩家碰撞
		POINT check_position = { position.x + (FRAME_WIDTH / 2), position.y + (FRAME_HEIGHT / 2) };
		bool is_overlap_x = (check_position.x > player.x && check_position.x < player.x + FRAME_WIDTH);
		bool is_overlap_y = (check_position.y > player.y && check_position.y < player.y + FRAME_HEIGHT);
		return is_overlap_x && is_overlap_y;
	}
	void move(const PLAYER& player) {
		player_x = player.x;
		player_y = player.y;
		
		int dir_x = player_x - position.x;
		int dir_y = player_y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0) {
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(SPEED * normalized_x);
			position.y += (int)(SPEED * normalized_y);
		}
	}
	void Draw(int delta) {
		int pos_shadow_x = position.x + (FRAME_WIDTH / 2 - 48 / 2);
		int pos_shadow_y = position.y + (FRAME_HEIGHT - 35);
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);
		if (facing_left) {
			anim_left->play(position.x, position.y, delta);
		}
		else {
			anim_right->play(position.x, position.y, delta);
		}
	}

};

void loadall() {
	AddFontResourceEx(_T("img/IPix.ttf"), FR_PRIVATE, NULL);
	loadimage(&button_game_begin, _T("img/ui_begin.png"), 192, 75);
	loadimage(&button_game_exit, _T("img/ui_exit.png"), 192, 75);
	loadimage(&ui_background, _T("img/ui_background.png"), 1280, 720);
	loadimage(&atlas_player_stand_right, _T("img/player_stand_right.png"), 50, 100);
	loadimage(&atlas_player_run_right[0], _T("img/player_run_right_0.png"), 103, 89);
	loadimage(&atlas_player_run_right[1], _T("img/player_run_right_1.png"), 77, 103);
	loadimage(&atlas_player_run_right[2], _T("img/player_run_right_2.png"), 107, 87);
	loadimage(&atlas_player_run_right[3], _T("img/player_run_right_3.png"), 78, 103);
	loadimage(&speak, _T("img/speak.png"), 1200, 200);
	loadimage(&white_all, _T("img/speak.png"), 1280, 720);
	int width = atlas_player_stand_right.getwidth();
	int height = atlas_player_stand_right.getheight();
	Resize(&atlas_player_stand_left, width, height);
	DWORD* color_buffer_left = GetImageBuffer(&atlas_player_stand_left);
	DWORD* color_buffer_right = GetImageBuffer(&atlas_player_stand_right);
	for (int yy = 0; yy < height; yy++) {
		for (int xx = 0; xx < width; xx++) {
			int idx_right = yy * width + xx;
			int idx_left = yy * width + (width - xx - 1);
			color_buffer_left[idx_left] = color_buffer_right[idx_right];
		}
	}
	for (int i = 0; i < NUMS; i++) {
		width = atlas_player_run_right[i].getwidth();
		height = atlas_player_run_right[i].getheight();
		Resize(&atlas_player_run_left[i], width, height);
		color_buffer_left = GetImageBuffer(&atlas_player_run_left[i]);
		color_buffer_right = GetImageBuffer(&atlas_player_run_right[i]);
		for (int yy = 0; yy < height; yy++) {
			for (int xx = 0; xx < width; xx++) {
				int idx_right = yy * width + xx;
				int idx_left = yy * width + (width - xx - 1);
				color_buffer_left[idx_left] = color_buffer_right[idx_right];
			}
		}
	}
	for (int i = 0; i < NUMS; i++) {
		static TCHAR img_path[256];
		_stprintf_s(img_path, _T("img/enemy_left_%d.png"), i);
		loadimage(&atlas_enemy_left[i], img_path);
		width = atlas_enemy_left[i].getwidth();
		height = atlas_enemy_left[i].getheight();
		Resize(&atlas_enemy_right[i], width, height);
		color_buffer_left = GetImageBuffer(&atlas_enemy_left[i]);
		color_buffer_right = GetImageBuffer(&atlas_enemy_right[i]);
		for (int yy = 0; yy < height; yy++) {
			for (int xx = 0; xx < width; xx++) {
				int idx_left = yy * width + xx;
				int idx_right = yy * width + (width - xx - 1);
				color_buffer_right[idx_right] = color_buffer_left[idx_left];
			}
		}
	}
};

void TryGenerateEnemy(std::vector<Enemy*>& enemy_list) {
	const int INTERVAL = 100;//生成间隔
	static int counter = 0;//计数器
	const int MAX_ENEMY = 10;//最大敌人数量
	static int ennum = 0;//敌人数
	if ((++counter % INTERVAL == 0) && (ennum < MAX_ENEMY)) {
		Enemy* enemy = new Enemy();
		enemy_list.push_back(enemy);//生成敌人并添加到列表
		ennum++;
		//MessageBox(GetHWnd(), _T("10！"), _T("游戏结束"), MB_OK);
	}
}

int main() {
	static std::vector<Enemy*> enemy_list;
	
	CIRCLE_GOD oo;//子弹	//要在主循环外面实例化对象，惨痛的教训QAQ
	HUANGFANG hf;
	PLAYER player;//玩家
	player.hp = 5;
	loadall();
	initgraph(1280, 720);

	mciSendString(_T("open mus/bgm.mp3 alias bgm"), NULL, 0, NULL);//播放背景音乐
	mciSendString(_T("open mus/hit.wav alias hit"), NULL, 0, NULL);//播放背景音乐
	mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL);

	settextstyle(28, 0, _T("IPix"));
	setbkmode(TRANSPARENT);
	BeginBatchDraw();
	while (running) {
		DWORD frame_strat_time = GetTickCount();
		if (scene_now == 0) {//主菜单场景
			static bool is_begin_down = false;
			static bool is_exit_down = false;
			while (peekmessage(&msg)) {
				if (msg.message == WM_LBUTTONDOWN) {//如果按下就标记
					if (msg.x > button_gamebegin_x - 5 && msg.x < button_gamebegin_x + 205 && msg.y > button_gamebegin_y - 5 && msg.y < button_gamebegin_y + 55) {
						is_begin_down = true;
					}
					if (msg.x > button_gameexit_x - 5 && msg.x < button_gameexit_x + 205 && msg.y > button_gameexit_y - 5 && msg.y < button_gameexit_y + 55) {
						is_exit_down = true;
					}
				}
				else if (msg.message == WM_LBUTTONUP) {//如果松开并且已经有标记就进入下一个场景，即按下开始游戏按钮
					if (msg.x > button_gamebegin_x - 5 && msg.x < button_gamebegin_x + 205 && msg.y > button_gamebegin_y - 5 && msg.y < button_gamebegin_y + 55) {
						if (is_begin_down) {
							scene_now = 1;//进入第一幕场景
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
			static bool is_first_come1 = true;
			if (is_first_come1) {
				player.x = 0;
				player.y = 380;
				is_first_come1 = false;
				for (int i = 0; i < 3; i++) {
					cleardevice();
					line(0, 480, 1280, 480);
					player.Draw();
					setfillcolor(RGB(158, 200, 231));
					solidellipse(0, 480, 1280, 720);
					for (int j = 0; j < sizeof(m1[i]); j++) {
						if (m1[i][j] == 'a')break;
						outtextxy(100 + (j % 30) * 30, 550 + (j / 30) * 30, m1[i][j]);
						FlushBatchDraw();
						Sleep(40);
					}
					settextstyle(20, 0, _T("IPix"));
					outtextxy(900, 660, _T("请按Enter健继续..."));
					FlushBatchDraw();
					settextstyle(28, 0, _T("IPix"));
					flushmessage();
					while (1) {
						getmessage(&msg);
						if (msg.message == WM_KEYDOWN && msg.vkcode == VK_RETURN) {
							break;
						}
					}
				}
			}
			while (peekmessage(&msg)) {
				player.Message_cheak(msg);
			}
			player.Move();
			if (player.x > 1280) {
				player.x = 0;
				player.y = 400;
				scene_now = 2;
			}
			static int counter = 0;
			if (++counter % NUMS == 0) {//更新程序帧以及动画帧
				idx_play++;
			}
			idx_play %= NUMS;
			cleardevice();
			line(0, 480, 1280, 480);
			player.Draw();
			FlushBatchDraw();
		}
		else if (scene_now == 2) {//对话，得到武器：圆
			static bool is_1_come2 = true;
			static bool o_fx = false;
			if (is_1_come2) {
				oo.oo_x = 1150;
				oo.oo_y = 320;
				player.is_right = false;
				player.x = 0;
				player.y = 380;
				is_1_come2 = false;
				for (int i = 0; i < 3; i++) {
					cleardevice();
					line(0, 480, 1280, 480);
					rectangle(1100, 380, 1200, 480);
					oo.o_draw();
					player.Draw();
					setfillcolor(RGB(158, 200, 231));
					solidellipse(0, 480, 1280, 720);
					for (int j = 0; j < sizeof(m2[i]); j++) {
						if (m2[i][j] == 'a')break;
						outtextxy(100 + (j % 30) * 30, 550 + (j / 30) * 30, m2[i][j]);
						FlushBatchDraw();
						Sleep(40);
					}
					settextstyle(20, 0, _T("IPix"));
					outtextxy(900, 660, _T("请按Enter健继续..."));
					FlushBatchDraw();
					settextstyle(28, 0, _T("IPix"));
					flushmessage();
					while (1) {
						getmessage(&msg);
						if (msg.message == WM_KEYDOWN && msg.vkcode == VK_RETURN) {
							break;
						}
					}
				}
			}

			while (peekmessage(&msg)) {
				player.Message_cheak(msg);
			}
			player.Move();
			if (player.x > 1020) {
				player.x = 1020;
			}
			if (player.x == 1020 && player.y == 380) {
				cleardevice();
				putimage(0, 0, &white_all);
				Sleep(100);
				FlushBatchDraw();
				scene_now = 3;
				player.is_right = false;
				continue;
			}
			static int counter = 0;
			if (++counter % NUMS == 0) {//更新程序帧以及动画帧
				idx_play++;
			}
			idx_play %= NUMS;

			if (o_fx) {
				oo.oo_y -= 1;
				if (oo.oo_y == 300) {
					o_fx = false;
				}
			}
			else {
				oo.oo_y += 1;
				if (oo.oo_y == 340) {
					o_fx = true;
				}
			}
			cleardevice();
			line(0, 480, 1280, 480);
			rectangle(1100, 380, 1200, 480);
			circle(oo.oo_x, oo.oo_y, oo.r);
			player.Draw();
			FlushBatchDraw();
		}
		else if (scene_now == 3) {//初战小怪（e）
			static bool is_tonext = false;
			static bool is_first_come3 = true;
			static bool first_attack = false;
			static int enemy_nums = 0;

			//加载敌人
			TryGenerateEnemy(enemy_list);
			//敌人移动
			for (Enemy* enemy : enemy_list)
				enemy->move(player);
			

			if (is_first_come3) {
				is_first_come3 = false;
				oo.With(player.x, player.y, last_fx);
				cleardevice();
				line(0, 480, 1280, 480);
				player.Draw();
				oo.o_draw();
				setfillcolor(RGB(158, 200, 231));
				solidellipse(0, 480, 1280, 720);
				for (int j = 0; j < sizeof(m3[0]); j++) {
					if (m3[0][j] == 'a')break;
					outtextxy(100 + (j % 30) * 30, 550 + (j / 30) * 30, m3[0][j]);
					FlushBatchDraw();
					Sleep(50);
				}
				settextstyle(20, 0, _T("IPix"));
				outtextxy(900, 660, _T("请按Enter健继续..."));
				FlushBatchDraw();
				settextstyle(28, 0, _T("IPix"));
				flushmessage();
				while (1) {
					getmessage(&msg);
					if (msg.message == WM_KEYDOWN && msg.vkcode == VK_RETURN) {
						break;
					}
				}
			}
			while (peekmessage(&msg)) {
				player.Message_cheak(msg);
			}
			player.Move();
			if (player.x > 1200) {
				if (is_tonext) {
					player.x = 0;
					player.y = 380;
					scene_now = 4;
				}
				else {
					player.x = 1200;
				}
			}
			if (player.is_attack && oo.attack_form == 0) {
				oo.attack_form = 1;
				oo.fx_attack = last_fx ? true : false;
			}

			static int counter = 0;
			if (++counter % NUMS == 0) {//更新程序帧以及动画帧
				idx_play++;
			}
			idx_play %= NUMS;
			cleardevice();
			line(0, 480, 1280, 480);
			if (oo.attack_form == 1) {
				oo.Do_attack();
			}
			else if (oo.attack_form == 2) {
				oo.Back(player.x + 40, player.y + 40);
				if (oo.attack_form == 0) {
					player.is_attack = false;
				}
			}
			else if (oo.attack_form == 0) {
				oo.With(player.x, player.y, last_fx);
			}
			player.Draw();
			oo.o_draw();

			//渲染敌人
			for (Enemy* enemy : enemy_list) {
				enemy->Draw(1000 / FPS);
			}

			//玩家敌人碰撞检测
			for (Enemy* enemy : enemy_list) {
				if (enemy->CheckPlayerCollision(player)) {
					MessageBox(GetHWnd(), _T("你被敌人干掉了！"), _T("游戏结束"), MB_OK);
					running = false;
					break;
				}
			}
			//敌人子弹碰撞检测
			for (Enemy* enemy : enemy_list) {
				if (enemy->CheckBulletCollision(oo))
				{
					//MessageBox(GetHWnd(), _T("攻击到了！"), _T("游戏结束"), MB_OK);
					enemy->Hurt();
					mciSendString(_T("play hit from 0"), NULL, 0, NULL);
				}
			}

			//敌人死亡
			for (int i = 0; i < enemy_list.size(); i++) {
				Enemy* enemy = enemy_list[i];
				if (!enemy->CheckAlived()) {
					std::swap(enemy_list[i], enemy_list.back());
					enemy_list.pop_back();
					delete enemy;
				}
			}

			if (enemy_list.size() == 0) {
				//没有敌人了，进入下一幕
			}

			FlushBatchDraw();
		}
		else if (scene_now == 4) {//解开幻方，营救刘徽
			static bool is_left = false;
			static bool is_first_come4 = true;
			static bool is_right = false;
			static bool is_attack = false;
			static int is_jump = 0;
			if (is_first_come4) {
				is_first_come4 = false;
				cleardevice();
				line(0, 480, 1280, 480);
				player.Draw();
				hf.on_draw();
				FlushBatchDraw();
				Sleep(50);
				for (int i = 0; i < 10; i++) {
					//对话。。。
				}
			}
			while (peekmessage(&msg)) {
				hf.message_cheak(msg);
			}
			static int counter = 0;
			if (++counter % NUMS == 0) {//更新程序帧以及动画帧
				idx_play++;
			}
			idx_play %= NUMS;
			cleardevice();
			hf.on_draw();
			FlushBatchDraw();
			if (hf.win_cheak()) {
				cleardevice();
				putimage(0, 0, &white_all);
				FlushBatchDraw();
				Sleep(50);
				scene_now = 5;
			}
		}
		else if (scene_now == 5) {
			cleardevice();
			FlushBatchDraw();
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