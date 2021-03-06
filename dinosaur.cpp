#include <string>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include <vector>
#include <fstream>
#include <algorithm>

using namespace std;

struct Coord {

	int x;
	int y;

	bool operator == (Coord &other) {
	
		return this->x == other.x && this->y == other.y;
	}
	Coord() {};
	Coord(int y, int x) {
	
		this->y = y;
		this->x = x;
	}
};

void readFileWithUnknownSize(vector<vector<char>> &ls, string file) {

	ifstream in;
	int height, width;
	in.open(file);
	in >> height;
	in >> width;

	ls.clear();
	ls.resize(height);
	vector<char> s;
	char c;
	for (int i = 0; i < (int)ls.size(); i++) {
	
		for (int j = 0; j < width; j++) {
		
			in >> noskipws >> c;
			s.push_back(c);
		}	
		ls[i] = s;
		s.clear();
	}
	in.close();
}

class Dinosaur {

private:
	Coord pivot;
	Coord org_pivot;
	vector<vector<char>> body_idle;
	vector<vector<char>> body_run1;
	vector<vector<char>> body_run2;
	vector<vector<char>> body_crouch;
	vector<vector<char>> body;

	const string asset_idle = "dino_idle.ast";
	const string asset_run1 = "dino_run1.ast";
	const string asset_run2 = "dino_run2.ast";
	const string asset_crouch = "dino_crouch.ast";

	int state_code = IDLE_STATE;
	int on_space = 0;
	int max_on_space = 2;
	int jump_height;
	int fall_speed = 1;
	bool crouchable = false;

	void readAssetFile() {

		readFileWithUnknownSize(body_idle, asset_idle);
		readFileWithUnknownSize(body_run1, asset_run1);
		readFileWithUnknownSize(body_run2, asset_run2);
		readFileWithUnknownSize(body_crouch, asset_crouch);
	}

public:
	static const int IDLE_STATE   = 100;
	static const int RUN1_STATE   = 111;
	static const int RUN2_STATE   = 122;
	static const int JUMP_STATE   = 133;
	static const int FALL_STATE   = 144;
	static const int CROUCH_STATE = 155;
	bool start = true;

	Dinosaur(int y, int x) {
	
		this->pivot.x = x;
		readAssetFile();
		body = body_idle;
		this->pivot.y = y - getHeight();
		org_pivot = pivot;	
		pivot.x = 0;
	}
	
	Coord getPivot() {
	
		return this->pivot;
	}
	
	void setPivotY(int y) {
	
		this->pivot.y = y;
	}
	
	int getHeight() {
	
		return this->body.size();
	}
	int getWidth() {
	
		return this->body[0].size();
	}

	bool onRightPos() {
	
		return pivot.x == org_pivot.x;
	}

	void setMaxOnSpace(int t) {
	
		this->max_on_space = t;
	}

	void setFallSpeed(int speed) {

		this->fall_speed = speed;
	}

	void Start() {
	
		Jump();
		pivot.x = 0;
		start = true;
	}

	void Crouch() {
	
		if (crouchable && this->state_code != JUMP_STATE && abs(pivot.y - org_pivot.y) <= 3) {
		
			setStateCode(CROUCH_STATE);
		}
	}

	void Jump() {

		if (this->state_code == JUMP_STATE)
			jump_height = org_pivot.y - getHeight() * 2;
		else if (this->state_code != FALL_STATE && this->state_code != JUMP_STATE) 
			setStateCode(JUMP_STATE), on_space = 0, jump_height = org_pivot.y - getHeight() * 1.4;
	}
	
	void riseIfNecessary() {

		if (fall_speed == 2) startFalling();
		else if (this->state_code == JUMP_STATE) {
			
			if (pivot.y <= jump_height) {
			
				pivot.y = jump_height;
				if (on_space == max_on_space) startFalling();
				else on_space++;
			}
			else pivot.y--;
		}
	}

	void startFalling() {
	
		setStateCode(FALL_STATE);
		on_space = 0;
	}
	
	void fallIfNecessary() {
	
		if (this->state_code == FALL_STATE || this->state_code == CROUCH_STATE) {
		
			if (pivot.y >= org_pivot.y) {
			
				pivot.y = org_pivot.y;
				if (this->state_code == FALL_STATE) 
					setStateCode((fall_speed == 2 && crouchable)? CROUCH_STATE: RUN1_STATE);
				setFallSpeed(1);
			}
			else pivot.y += fall_speed;
		}
		if ((state_code == RUN1_STATE || state_code == RUN2_STATE) && pivot.x < org_pivot.x) pivot.x++;
		if (pivot.x == org_pivot.x) crouchable = true;
	}

	void toggleStep() {
	
		if (state_code == RUN1_STATE) setStateCode(RUN2_STATE);
		else if (state_code == RUN2_STATE) setStateCode(RUN1_STATE);
	}

	int getStateCode() {
	
		return this->state_code;
	}

	void setStateCode(int code) {
	
		this->state_code = code;

		if (state_code == IDLE_STATE)   body = body_idle;
		if (state_code == JUMP_STATE)   body = body_idle;
		if (state_code == FALL_STATE)   body = body_idle;
		if (state_code == RUN1_STATE)   body = body_run1;
		if (state_code == RUN2_STATE)   body = body_run2;
		if (state_code == CROUCH_STATE) body = body_crouch;
	}

	vector<vector<char>> getBody() {
	
		return this->body;
	}	
};

class Obstacle {

private:
	vector<vector<char>> body;
	string asset_file;
	Coord pivot;
	int map_height, map_width;

	void readAssetFile() {
	
		readFileWithUnknownSize(body, asset_file);
	}

public:

	Obstacle();
	Obstacle(string asset_file, int map_height, int map_width, int org_x) {

		this->map_height = map_height;
		this->map_width = map_width;
		this->asset_file = asset_file;
		this->pivot.x = org_x;
		readAssetFile();
		this->pivot.y = map_height - getHeight() - 3;
	}
	
	string getAssetFileName() {
	
		return this->asset_file;
	}
	void setAssetFile(string file) {
	
		this->asset_file = file;
	}
	Coord getPivot() {
	
		return this->pivot;
	}
	void setPivotX(int x) {
	
		this->pivot.x = x;
	}

	int getHeight() {
	
		return this->body.size();
	}
	int getWidth() {
	
		return this->body[0].size();
	}
	bool Move() {

		pivot.x--;
		if (pivot.x + getWidth() - 1 <= 0) return true;
		return false;
	}
	bool insideMap(int i, int j) {

		return pivot.y + i < map_height && pivot.y + i >= 0 && pivot.x + j < map_width && pivot.x + j >= 0;
	}
	vector<vector<char>> getBody() {
	
		return this->body;
	}
};

class Land {

public:
	int length;
	bool start;
	string layer0, layer0p;
	string layer1, layer1p;
	string layer2, layer2p;
	
	int pos_line;
	int curr = 1;

	void createDotLayer(string &layer, int len, double spawn_rate) {
	
		int dot_count = len * spawn_rate;
		int c = 0;
		layer = "";
		for (int i = 0; i < len; i++) {
		
			int rd = rand() % 8;
			layer += (rd == 0 && c < dot_count)? ".":" ";
			c += rd == 0;
		}
	}

	void createFloor(string &layer, int len) {
	
		layer = "";
		int odd_count = 2;
		int c = 0;
		for (int i = 0; i < len; i++) { 
			int rd = rand() % 40;
			layer += (c < odd_count && rd == 0)? "^": "=";
			c += rd == 0;
		}
	}

public:
	void makeAllLayer(bool df = true, int dfl = 10){

		if (df) dfl = length;

		createFloor(layer0, dfl);
		createFloor(layer0p, dfl * 2);

		createDotLayer(layer1, dfl, dfl / 6.);
		createDotLayer(layer1p, dfl, dfl / 6.);
		createDotLayer(layer2, dfl, dfl / 10.);
		createDotLayer(layer2p, dfl, dfl / 10.);
	}

	void Move() {

		layer0 += layer0p[0]; char temp = layer0[0]; layer0 = layer0.substr(1); layer0p = layer0p.substr(1); layer0p = temp + layer0p;
		layer1 += layer1p[0];      temp = layer1[0]; layer1 = layer1.substr(1); layer1p = layer1p.substr(1); layer1p = temp + layer1p;
		layer2 += layer2p[0];      temp = layer2[0]; layer2 = layer2.substr(1); layer2p = layer2p.substr(1); layer2p = temp + layer2p;
	}

	void Draw() {
	
		mvaddstr(pos_line,     0, const_cast<char *>(layer0.c_str()));
		mvaddstr(pos_line + 1, 0, const_cast<char *>(layer1.c_str()));
		mvaddstr(pos_line + 2, 0, const_cast<char *>(layer2.c_str()));
	}

	void setPos(int pos) {
	
		this->pos_line = pos;
	}

	Land() {
	
		makeAllLayer(false, length / 10);
	}

	Land(int length, int pos_line, int isStarted) {
	
		this->length = length;
		this->pos_line = pos_line;
		this->start = isStarted;
		if (!isStarted) makeAllLayer(false, length / 10);
		else makeAllLayer(false, length);
	}
};

class GameManager {

private:
	int map_width, map_height;
	unsigned long long highscore = 0;
	unsigned long beat = 0;

	Land *land;
	Dinosaur *dino;
	vector<Obstacle*> obs_list;

	vector<string> map, blank_map; 
	
	const string highscore_file = "highscore.txt";
	bool keyPressed = false;
	int difficult = 1;
	int clock = 10000;
	time_t t_start, t_now;

public:
	bool isGameOver = false;
	bool isStarted = false;
	bool isExit = false;
	GameManager();
	GameManager(int height, int width) {

		this->map_height = height;
		this->map_width = width;
		string s(map_width, ' ');
		blank_map.clear();
		for (int i = 0; i < map_height; i++) blank_map.push_back(s);

	}
	~GameManager() {
	
		delete dino;
		delete land;
		for (int i = 0; i < (int)obs_list.size(); i++) delete obs_list[i];
	}

	void changeState() {
	
		time(&t_now);
		land->Move();
		for (int i = 0; i < (int)obs_list.size(); i++) {
		
			if (obs_list.at(i)->Move()) {
		
				Obstacle *temp = obs_list.at(i);
				obs_list.erase(obs_list.begin() + i, obs_list.begin() + i + 1);
				temp->setPivotX(obs_list.back()->getPivot().x + obs_list.back()->getWidth() + 50 + rand() % (map_width / 2));
				obs_list.push_back(temp);
			}
		}
		
		if (difftime(t_now, t_start) + 1 >= 15) {
			difficult++;
			difficult = min(8, difficult);
			time(&t_start);
		}
	
		char key = getch();
		if (key == 's') {
			
			if (dino->getStateCode() == Dinosaur::FALL_STATE || dino->getStateCode() == Dinosaur::JUMP_STATE) {
				dino->setFallSpeed(2);
			}
			else if (dino->getStateCode() == Dinosaur::CROUCH_STATE) dino->setStateCode(Dinosaur::RUN1_STATE);
			else dino->Crouch();
		}
		else if (key == ' ' && !dino->start) dino->Jump();
		else if (key == 'q') isExit = true;

		if (dino->start && dino->onRightPos()) 
			land->makeAllLayer(), dino->start = false;;
		
		if (beat % 8 == 0) dino->toggleStep();
		if (beat % 2 == 0) dino->riseIfNecessary();
		if (beat % 2 == 0) dino->fallIfNecessary();

		beat++;
		dino->setMaxOnSpace(2 + min(6, difficult));
		clock = max(2000, 10000 - difficult*800);
		highscore = (beat / 5 > highscore)? beat / 5 : highscore;
	}

	void setDifficult(int diff) {
	
		this->difficult = diff;
	}

	void Render() {

	    map = blank_map;		
		for (int j = 0; j < map_width; j++) {
			
			map[map_height - 3][j] = (j < (int)land->layer0.length())? land->layer0[j] : ' ';
			map[map_height - 2][j] = (j < (int)land->layer1.length())? land->layer1[j] : ' ';
			map[map_height - 1][j] = (j < (int)land->layer2.length())? land->layer2[j] : ' ';
		}

		for (int i = 0; i < dino->getHeight(); i++) {
		
			for (int j = 0; j < dino->getWidth(); j++) {
	
				map[dino->getPivot().y + i][dino->getPivot().x + j] = dino->getBody()[i][j];
			}
		}
		
		if (isStarted)
		for (Obstacle *obs : obs_list)
		for (int i = 0; i < obs->getHeight(); i++) {
		
			for (int j = 0; j < obs->getWidth(); j++) {
			
				if (obs->insideMap(i, j)) {
					if (map[obs->getPivot().y + i][obs->getPivot().x + j] == ' ')
						map[obs->getPivot().y + i][obs->getPivot().x + j] = obs->getBody()[i][j];
					else if (map[obs->getPivot().y + i][obs->getPivot().x + j] == '=' && obs->getBody()[i][j] != ' ') {
						isGameOver = true;
					}
				}
			}
		}
	
		for (int i = 0; i < map_height; i++) {
			
			for (int j = 0; j < map_width; j++) {
			
				mvaddch(i, j, map[i][j]);
			}
		}
		mvprintw(1, map_width - 15, "HIGH: %d           ", highscore);
		mvprintw(2, map_width - 15, "CURR: %d           ", beat / 5);
	}

	void Init() {
		
		dino = new Dinosaur(map_height - 3, 10);
	
		obs_list.clear();
		for (int i = 0; i < 20; i++) {

			string asset;
			int type = rand() % 4 + 1;
			obs_list.push_back(new Obstacle("cactus" + to_string(type) + ".ast", map_height, map_width, map_width + 100 + 100 * i - rand() % 25));
			mvprintw(i, 0, "%d     ", obs_list[i]->getPivot().x);
		}

		land = new Land(map_width, map_height, isStarted);
		beat = 0;
		isGameOver = false;

		ifstream in; in.open("highscore.dat");
		in >> highscore;

		Render();
		mvaddstr(map_height / 2 - 3, map_width / 2 - 8, "NO INTERNET");
		mvaddstr(map_height / 2 - 2, map_width / 2 -  12, "Press space to play");
		
		char key = getch();
		while (key != ' ') {

			if (key == 'q') endwin(), exit(0);
			key = getch();
		} 
		dino->Start();	
		time(&t_start);
		difficult = 1;
		isStarted = true;
	}

	void Play() {
		
		while (true) {
		
			Init();
			while(!isGameOver) {
			
				changeState();
				Render();
				usleep(clock);
				refresh();
				if (isExit) goto Exit;
			}
			for (Obstacle *obs: obs_list) obs->Move();
			Render();
			mvaddstr(map_height / 2 - 3, map_width / 2 - 6, "GAME OVER");
			mvaddstr(map_height / 2 - 2, map_width / 2 - 12, "Press space to replay");
			mvaddstr(map_height / 2 - 1, map_width / 2 - 9,  "Press Q to quit");
			ofstream out; out.open("highscore.dat");
			out << highscore;
			char key;
			while (true) {
		
				usleep(20000);
				key = getch();
				if (key == ' ') break;
				else if (key == 'q') goto Exit;
			}
			delete dino;
			for (Obstacle *obs: obs_list) delete obs;
		}
Exit: { }
	}
};

int main() {

	initscr();
	raw();
	curs_set(0);
	keypad(stdscr, 1);
	cbreak();
	noecho();
	scrollok(stdscr, TRUE);
   	nodelay(stdscr, TRUE);
	
	GameManager gameManager(50, 200);
	gameManager.Play();
	
	endwin();
	return 0;
}
