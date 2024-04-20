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
const int FPS = 50;//֡��
const int NUMS = 4;
//const int frame_conversion = 7;
const int button_gamebegin_x = 550;//��ʼ��Ϸ��ť����
const int button_gamebegin_y = 400;
const int button_gameexit_x = 550;//�˳���Ϸ��ť����
const int button_gameexit_y = 500;
int scene_now = 0;//Ŀǰ����һĻ
int idx_play = 0;//����֡
int last_fx = 0;//�����һ�γ���
bool running = true;
ExMessage msg;
IMAGE button_game_begin;
IMAGE button_game_exit;
IMAGE player;
IMAGE ui_background;//���˵�����
IMAGE atlas_player_stand_right;//���վ������ͼ��
IMAGE atlas_player_stand_left;//���վ������ͼ��
IMAGE atlas_player_run_right[NUMS];//��ұ��ܳ���ͼ��
IMAGE atlas_player_run_left[NUMS];//��ұ��ܳ���ͼ��
IMAGE atlas_enemy_right[NUMS];//���˳��ұ���ͼ��
IMAGE atlas_enemy_left[NUMS];//���˳�����ͼ��
IMAGE atlas_speak[NUMS];//�Ի�ͼ��
IMAGE speak;
IMAGE white_all;
TCHAR m1[3][100] = { _T("��������: ���Ǻ��ˣ�Ϊ�λ����ˣ�a"),_T("��:...a"),_T("��������: �ţ�����˵��ȴ����ʵ�壬�ѵ�...��Ȥ��������Ȥ���������ܰﵽ����...���ɣ���ǰ�߼���������ܿ������ǣ�����Բ�ˡ�a") };
TCHAR m2[3][100] = { _T("��������: ����������˵�����Բ������������ΰ������أ��������Ĵ��ڡ������ȴ�����������������...a"),_T("����δ�䣬�����������...a"),_T("��������: ������찡���������ˣ��쿿��Բ�����份�ѣ��һ����ģ������������ˣ���!a") };
TCHAR m3[3][100] = { _T("��������: ���������ǳ����õ���Բ���Ͽ��ˣ��ٿ�Բ����Щ�����ɵ���a"),_T("��������: ���������û�ͦ�졣���ˣ��������ҽ����ˣ�����Բ�Ĵ�����֮һ������Խ���--���֮��a"),_T("��...��Ȼ���ܾȳ�Բ����һ��Ҳ�ܾȳ���һλ�����ˣ��ҵ�ǰ��--���ա�ǰ���ɣ�һ��ȥ�ȳ�����������ջ�ġ�a") };

inline void putimage_alpha(int x, int y, IMAGE* img) {//�ܴ���͸����������Ⱦ����
	int w = img->getwidth();
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h,
		GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });
}

//����������
class Animation {
private:
	std::vector<IMAGE*> frame_list;	//����֡�б�
	int interval_ms=0;//�������
	int timer = 0;//������ʱ��
	int idx_frame = 0;//����֡����
public:
	Animation(LPCTSTR path, int num, int interval) {
		interval_ms = interval;
		TCHAR path_file[256];
		for (int i = 0; i <= num; i++) {
			_stprintf_s(path_file, path, i);
			IMAGE* frame = new IMAGE();
			loadimage(frame, path_file);
			frame_list.push_back(frame);//��Ӷ���֡
		}
	}
	~Animation() {
		for (auto& frame : frame_list) {
			delete frame;//�ͷŶ���֡
		}
	}

	void play(int x, int y, int delta) {
		timer += delta;//���¼�ʱ��
		if (timer >= interval_ms) {
			idx_frame = (idx_frame + 1) % frame_list.size();//�л���һ֡
			timer = 0;//���ü�ʱ��
		}
		putimage_alpha(x, y, frame_list[idx_frame]);//��Ⱦ����֡
	}
};


//����Բ��
class CIRCLE_GOD {
public:
	CIRCLE_GOD() = default;
	~CIRCLE_GOD() = default;
	void o_draw() {//��Բ
		circle(oo_x, oo_y, r);
	}
	void Do_attack() {//����
		if (fx_attack) {
			oo_x -= oo_speed;
		}
		else {
			oo_x += oo_speed;
		}
		oo_attack_dist++;
		if (oo_attack_dist > 15) {//����ﵽ���������룬�ͷ����������
			attack_form = 2;
		}
	}
	void With(int x, int y, int fx) {//��������
		if (fx) {
			oo_x = x - 20;
			oo_y = y + 50;
		}
		else {
			oo_x = x + 100;
			oo_y = y + 50;
		}
	}
	void Back(int x, int y) {//�����������
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
	int oo_form = 0;//Բ��ǰ��̬
	int oo_x;
	int oo_y;
	int r = 15;
	int oo_speed = 15;//�ٶ�
	int oo_attack_dist = 0;//��������
	int attack_form = 0;//���������еĽ׶�
	bool fx_attack = false;//��������
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
	bool win_cheak() {//�жϻ÷��Ƿ����
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

	void Draw() {//�������Ѫ��
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
	void Message_cheak(const ExMessage& img) {//������Ϣ
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
	void Move() {//�����ƶ�����Ծ
		if (is_left) {
			x -= speed;
			if (x < -30)x = -30;
			last_fx = 1;
		}
		if (is_right) {
			x += speed;
			last_fx = 0;
		}
		if (is_jump) {//��Ծ
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
	int speed = 5;//��ұ����ٶ�
	int hp;
};


//������
class Enemy {
private:
	const int SPEED = 2;		//�����ٶ�	
	const int FRAME_WIDTH = 44;//���˿��
	const int FRAME_HEIGHT = 50;//���˸߶�
	//const int SHADOW_WIDTH = 48;//��Ӱ

	int player_x;
	int player_y;
	IMAGE img_shadow;
	Animation* anim_left;//
	Animation* anim_right;//
	POINT position = { 0,0 };//λ��
	bool facing_left = false;//����
	bool alived = true;//���
public:
	Enemy() {
		loadimage(&img_shadow, _T("img/shadow_enemy.png"));//��Ӱ
		anim_left = new Animation(_T("img/e%d.png"), 4, 1000/FPS);//�󶯻���4�ţ����
		anim_right = new Animation(_T("img/e%d.png"), 4, 1000/FPS);//�Ҷ�����4�ţ����


		//���˳�ʼλ��
		enum class SpawnEdge {
			//UP = 0,
			//DOWN,
			LEFT=0,
			RIGHT,
		};
		//������ɵ���λ��
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
		//Բ���������ж���ײ
		bool is_overlap_x = ((bullet.oo_x > position.x) && (bullet.oo_x < position.x + FRAME_WIDTH));
		bool is_overlap_y = ((bullet.oo_y > position.y - FRAME_HEIGHT) && (bullet.oo_y < position.y + FRAME_HEIGHT));
		return is_overlap_x && is_overlap_y;

	}
	bool CheckPlayerCollision(const PLAYER& player) {
		//�����ײ
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
	const int INTERVAL = 100;//���ɼ��
	static int counter = 0;//������
	const int MAX_ENEMY = 10;//����������
	static int ennum = 0;//������
	if ((++counter % INTERVAL == 0) && (ennum < MAX_ENEMY)) {
		Enemy* enemy = new Enemy();
		enemy_list.push_back(enemy);//���ɵ��˲���ӵ��б�
		ennum++;
		//MessageBox(GetHWnd(), _T("10��"), _T("��Ϸ����"), MB_OK);
	}
}

int main() {
	static std::vector<Enemy*> enemy_list;
	
	CIRCLE_GOD oo;//�ӵ�	//Ҫ����ѭ������ʵ�������󣬲�ʹ�Ľ�ѵQAQ
	HUANGFANG hf;
	PLAYER player;//���
	player.hp = 5;
	loadall();
	initgraph(1280, 720);

	mciSendString(_T("open mus/bgm.mp3 alias bgm"), NULL, 0, NULL);//���ű�������
	mciSendString(_T("open mus/hit.wav alias hit"), NULL, 0, NULL);//���ű�������
	mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL);

	settextstyle(28, 0, _T("IPix"));
	setbkmode(TRANSPARENT);
	BeginBatchDraw();
	while (running) {
		DWORD frame_strat_time = GetTickCount();
		if (scene_now == 0) {//���˵�����
			static bool is_begin_down = false;
			static bool is_exit_down = false;
			while (peekmessage(&msg)) {
				if (msg.message == WM_LBUTTONDOWN) {//������¾ͱ��
					if (msg.x > button_gamebegin_x - 5 && msg.x < button_gamebegin_x + 205 && msg.y > button_gamebegin_y - 5 && msg.y < button_gamebegin_y + 55) {
						is_begin_down = true;
					}
					if (msg.x > button_gameexit_x - 5 && msg.x < button_gameexit_x + 205 && msg.y > button_gameexit_y - 5 && msg.y < button_gameexit_y + 55) {
						is_exit_down = true;
					}
				}
				else if (msg.message == WM_LBUTTONUP) {//����ɿ������Ѿ��б�Ǿͽ�����һ�������������¿�ʼ��Ϸ��ť
					if (msg.x > button_gamebegin_x - 5 && msg.x < button_gamebegin_x + 205 && msg.y > button_gamebegin_y - 5 && msg.y < button_gamebegin_y + 55) {
						if (is_begin_down) {
							scene_now = 1;//�����һĻ����
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
			putimage_alpha(0, 0, &ui_background);//������
			putimage_alpha(button_gamebegin_x, button_gamebegin_y, &button_game_begin);//����ʼ��Ϸ��ťͼ��
			putimage_alpha(button_gameexit_x, button_gameexit_y, &button_game_exit);//���˳���Ϸ��ťͼ��
			FlushBatchDraw();
		}
		else if (scene_now == 1) {//��һĻ��ʵ�ֶԻ�����Ծ������
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
					outtextxy(900, 660, _T("�밴Enter������..."));
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
			if (++counter % NUMS == 0) {//���³���֡�Լ�����֡
				idx_play++;
			}
			idx_play %= NUMS;
			cleardevice();
			line(0, 480, 1280, 480);
			player.Draw();
			FlushBatchDraw();
		}
		else if (scene_now == 2) {//�Ի����õ�������Բ
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
					outtextxy(900, 660, _T("�밴Enter������..."));
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
			if (++counter % NUMS == 0) {//���³���֡�Լ�����֡
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
		else if (scene_now == 3) {//��սС�֣�e��
			static bool is_tonext = false;
			static bool is_first_come3 = true;
			static bool first_attack = false;
			static int enemy_nums = 0;

			//���ص���
			TryGenerateEnemy(enemy_list);
			//�����ƶ�
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
				outtextxy(900, 660, _T("�밴Enter������..."));
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
			if (++counter % NUMS == 0) {//���³���֡�Լ�����֡
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

			//��Ⱦ����
			for (Enemy* enemy : enemy_list) {
				enemy->Draw(1000 / FPS);
			}

			//��ҵ�����ײ���
			for (Enemy* enemy : enemy_list) {
				if (enemy->CheckPlayerCollision(player)) {
					MessageBox(GetHWnd(), _T("�㱻���˸ɵ��ˣ�"), _T("��Ϸ����"), MB_OK);
					running = false;
					break;
				}
			}
			//�����ӵ���ײ���
			for (Enemy* enemy : enemy_list) {
				if (enemy->CheckBulletCollision(oo))
				{
					//MessageBox(GetHWnd(), _T("�������ˣ�"), _T("��Ϸ����"), MB_OK);
					enemy->Hurt();
					mciSendString(_T("play hit from 0"), NULL, 0, NULL);
				}
			}

			//��������
			for (int i = 0; i < enemy_list.size(); i++) {
				Enemy* enemy = enemy_list[i];
				if (!enemy->CheckAlived()) {
					std::swap(enemy_list[i], enemy_list.back());
					enemy_list.pop_back();
					delete enemy;
				}
			}

			if (enemy_list.size() == 0) {
				//û�е����ˣ�������һĻ
			}

			FlushBatchDraw();
		}
		else if (scene_now == 4) {//�⿪�÷���Ӫ������
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
					//�Ի�������
				}
			}
			while (peekmessage(&msg)) {
				hf.message_cheak(msg);
			}
			static int counter = 0;
			if (++counter % NUMS == 0) {//���³���֡�Լ�����֡
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