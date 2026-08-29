#include "myTool.hpp"
#include "snake.hpp"
#include <string>

namespace snake {
	namespace {
		class Setting {
		private:
			/*
			遊戲設定
			*/
			short _column = 10, _row = 10, _score = 0, _moveWaitTime = 500, _LShiftWaitTime = 250,
				_foodNum = 2, _mapArea = _row * _column, _maxScore = _mapArea - 1;
			bool _useWASD = true;
			/*
			上下限定義、常數
			*/
			const short _minColumn = 1, _maxColumn = 30, _minRow = 1, _maxRow = 30,
				_minWaitTime = 200, _maxWaitTime = 1000, _minFoodNum = 1, _maxFoodNum = 10,
				_tabColumn = 0, _tabRow = 0;
		public:
			short getColumn() const{
				return _column;
			}
			short getRow() const{
				return _row;
			}
			short getTabColumn() const{
				return _tabColumn;
			}
			short getTabRow() const{
				return _tabRow;
			}
			short getScore() const{
				return _score;
			}
			short getMoveWaitTime() const{
				return _moveWaitTime;
			}
			short getLShiftWaitTime() const{
				return _LShiftWaitTime;
			}
			short getFoodNum() const{
				return _foodNum;
			}
			short getMapArea() const{
				return _mapArea;
			}
			short getMaxScore() const{
				return _maxScore;
			}
			short getUseWASD() const{
				return _useWASD;
			}
			void setColumn(const short column) {
				myTool::myAssert(_minColumn <= column && column <= _maxColumn);
				_column = column;
			}
			void setRow(const short row) {
				myTool::myAssert(_minRow <= row && row <= _maxRow);
				_row = row;
			}
			void setScore(const short score) {
				myTool::myAssert(score <= _maxScore);
				_score = score;
			}
			void setMoveWaitTime(const short moveWaitTime) {
				myTool::myAssert(_minWaitTime <= moveWaitTime && moveWaitTime <= _maxWaitTime);
				_moveWaitTime = moveWaitTime;
			}
			void setLShiftWaitTime(const short LShiftWaitTime) {
				myTool::myAssert(_minWaitTime <= LShiftWaitTime && LShiftWaitTime <= _maxWaitTime);
				_LShiftWaitTime = LShiftWaitTime;
			}
			void setFoodNum(const short foodNum) {
				myTool::myAssert(_minFoodNum <= foodNum && foodNum <= _maxFoodNum);
				_foodNum = foodNum;
			}
			void updataMapArea() {
				_mapArea = _row * _column;
			}
			void updataMaxScore() {
				_maxScore = _mapArea - 1;
			}
			void setUseWASD(const bool state) {
				_useWASD = state;
			}
			void init() {
				setScore(0);
			}
		};
		Setting setting;

		class SnakeGame {
		private:
			struct Node {
				short x = 0, y = 0;
				Node* next = nullptr;
				Node() = default;
				Node(const short inX, const short inY) :x(inX), y(inY){}
				void setXY(const short inX, const short inY) {
					x = inX;
					y = inY;
				}
			};
			/*
			{以索引值(下標)儲存資訊}
			儲存已被佔用的地圖格子(0 ~ (setting::getMapArea - 1))
			方便找到未被佔用的地圖格子
			*/
			myTool::Tree _tree;
			/*
			蛇頭指標
			*/
			Node* _root = nullptr;
			/*
			{預計以 std::vector 來優化}
			食物位置指標
			*/
			Node* _foodBox[10] = { nullptr };
			/*
			(_foodBoxNum)與(_foodBox[])配合，儲存果子應持的數量與避免索引溢出

			{以索引值(下標)儲存資訊}
			(_remainingIndex)儲存目前尚有幾個空格子未用，主要用在(initSnakeGame()的 num 中)
			*/
			short _foodBoxNum = 0, _remainingIndex = 0,
				_xMove = 0, _yMove = -1;

			void deleteSnakeGame() {
				if (_root != nullptr) {
					Node* temp = nullptr;
					do{
						temp = _root->next;
						delete(_root);
						_root = temp;
					}while (_root != nullptr);
					_root = nullptr;
				}
				_tree.deleteTree();
				_remainingIndex = 0;

				for (short i = 0;i < _foodBoxNum;i++) {
					if (_foodBox[i] == nullptr) {
						continue;
					}
					delete(_foodBox[i]);
					_foodBox[i] = nullptr;
				}
				_foodBoxNum = 0;
				_xMove = 0;
				_yMove = -1;
			}
		public:
			void initSnakeGame(const short x = (short)(setting.getColumn() / 2), const short y = (short)(setting.getRow() / 2),
				const short num = (setting.getMaxScore() < setting.getFoodNum()) ? setting.getMaxScore() : setting.getFoodNum()) {
				deleteSnakeGame();
				/*
				以輸入的(x,y)建立與顯示頭部位置
				{預設置中(或偏右下)}
				*/
				myTool::myAssert(0 <= x && x < setting.getColumn() &&
					0 <= y && y < setting.getRow());
				_remainingIndex = setting.getMaxScore();
				_root = new Node(x, y);
				myTool::myCout("● ", setting.getTabColumn() + 2 + (x * 2), setting.getTabRow() + 3 + y, 1);
				_tree.insert(x + (y * setting.getColumn()));
				_remainingIndex--;

				/*
				以輸入的(num)選擇要初始化與隨機位置的果子數量
				*/
				myTool::myAssert(num <= 10);
				_foodBoxNum = num;
				for (short i = 0, randIndex = 0, x = 0, y = 0;i < _foodBoxNum;i++) {
					randIndex = myTool::myRand(0, _remainingIndex);
					_tree.findCanUseIndex(randIndex);
					x = randIndex % setting.getColumn();
					y = (short)(randIndex / setting.getColumn());
					_foodBox[i] = new Node(x, y);
					myTool::myCout("● ", setting.getTabColumn() + 2 + (x * 2), setting.getTabRow() + 3 + y, 4);
					_tree.insert(randIndex);
					_remainingIndex--;
				}
			}

			bool willGameOver(const short num) const{
				if (num == 0) {
					return false;
				}

				short mapColumn = 3 + (setting.getColumn() * 2), mapRow = 4 + setting.getRow();
				myTool::clearCmd(setting.getTabColumn(), setting.getTabRow(), (47 < mapColumn) ? mapColumn : 47, mapRow);
				switch (num) {
				case 1:
					myTool::myCout("以Esc退出\n\n最後得分: " +
						std::to_string(setting.getScore()) +
						"\n\n按空白鍵返回..", 0, 0, 7);
					while (true) {
						myTool::resetKeyInput();
						myTool::mySleep(200);
						if (myTool::getKeyInput(0x20)) {
							break;
						}
					}
					myTool::clearCmd(0, 0, 13, 4);
					return true;
					break;
				case 2:
					myTool::myCout("蛇撞牆\n\n最後得分: " +
						std::to_string(setting.getScore()) +
						"\n\n按空白鍵返回..", 0, 0, 7);
					while (true) {
						myTool::resetKeyInput();
						myTool::mySleep(200);
						if (myTool::getKeyInput(0x20)) {
							break;
						}
					}
					myTool::clearCmd(0, 0, 13, 4);
					return true;
					break;
				case 3:
					myTool::myCout("蛇咬到自己\n\n最後得分: " +
						std::to_string(setting.getScore()) +
						"\n\n按空白鍵返回..", 0, 0, 7);
					while (true) {
						myTool::resetKeyInput();
						myTool::mySleep(200);
						if (myTool::getKeyInput(0x20)) {
							break;
						}
					}
					myTool::clearCmd(0, 0, 13, 4);
					return true;
					break;
				case 4:
					myTool::myCout("蛇佔滿整個地圖，遊戲勝利\n\n最後得分: " +
						std::to_string(setting.getScore()) +
						"\n\n按空白鍵返回..", 0, 0, 7);
					while (true) {
						myTool::resetKeyInput();
						myTool::mySleep(200);
						if (myTool::getKeyInput(0x20)) {
							break;
						}
					}
					myTool::clearCmd(0, 0, 24, 4);
					return true;
					break;
				}
				myTool::myAssert(false);
				return true;
			}
			short willSnakeDie(const short nextX, const short nextY) const{
				if ((nextX < 0 || setting.getColumn() <= nextX)||
					(nextY < 0 || setting.getRow() <= nextY)) {
					return 2;
				}
				
				Node* temp = _root;
				while (temp != nullptr) {
					if (temp->x == nextX && temp->y == nextY) {
						return 3;
					}
					else {
						temp = temp->next;
					}
				}
				return 0;
			}
			bool snakeMove(const bool useWASD = setting.getUseWASD()) {
				if (setting.getMaxScore() <= setting.getScore()) {
					willGameOver(4);
					return false;
				}

				short waitTime = 0;
				if (myTool::getKeyInput(0xA0)) {
					waitTime = setting.getLShiftWaitTime();
				}
				else {
					waitTime = setting.getMoveWaitTime();
				}
				myTool::resetKeyInput();
				myTool::mySleep(waitTime);
				if (myTool::getKeyInput('0x1B')) {
					willGameOver(1);
					return false;
				}
				if (useWASD) {
					if (myTool::getKeyInput('W') && _yMove != 1) {
						_xMove = 0;
						_yMove = -1;
					}
					else if (myTool::getKeyInput('D') && _xMove != -1) {
						_xMove = 1;
						_yMove = 0;
					}
					else if (myTool::getKeyInput('S') && _yMove != -1) {
						_xMove = 0;
						_yMove = 1;
					}
					else if (myTool::getKeyInput('A') && _xMove != 1) {
						_xMove = -1;
						_yMove = 0;
					}
				}
				else {
					if (myTool::getKeyInput('0x26') && _yMove != 1) {
						_xMove = 0;
						_yMove = -1;
					}
					else if (myTool::getKeyInput('0x27') && _xMove != -1) {
						_xMove = 1;
						_yMove = 0;
					}
					else if (myTool::getKeyInput('0x28') && _yMove != -1) {
						_xMove = 0;
						_yMove = 1;
					}
					else if (myTool::getKeyInput('0x25') && _xMove != 1) {
						_xMove = -1;
						_yMove = 0;
					}
				}

				short nextX = _root->x + _xMove, nextY = _root->y + _yMove;
				if (willGameOver(willSnakeDie(nextX, nextY))) {
					return false;
				}

				for (short i = 0;i < _foodBoxNum;i++) {
					if (_foodBox[i] == nullptr) {
						continue;
					}
					if (_foodBox[i]->x == nextX && _foodBox[i]->y == nextY) {
						_foodBox[i]->next = _root;
						_root = _foodBox[i];
						_foodBox[i] = nullptr;
						myTool::myCout("● ", setting.getTabColumn() + 2 + (_root->x * 2), setting.getTabRow() + 3 + _root->y, 1);
						myTool::myCout("● ", setting.getTabColumn() + 2 + (_root->next->x * 2), setting.getTabRow() + 3 + _root->next->y, 2);

						if (0 <= _remainingIndex) {
							short randIndex = myTool::myRand(0, _remainingIndex), x = 0, y = 0;
							_tree.findCanUseIndex(randIndex);
							x = randIndex % setting.getColumn();
							y = (short)(randIndex / setting.getColumn());
							_foodBox[i] = new Node(x, y);
							myTool::myCout("● ", setting.getTabColumn() + 2 + (x * 2), setting.getTabRow() + 3 + y, 4);
							_tree.insert(randIndex);
							_remainingIndex--;
						}
						setting.setScore(setting.getScore() + 1);
						return true;
					}
				}

				Node* temp = _root;
				if (temp->next == nullptr) {
					_tree.remove(temp->x + (temp->y * setting.getColumn()));
					myTool::myCout("  ", setting.getTabColumn() + 2 + (temp->x * 2), setting.getTabRow() + 3 + temp->y);
					temp->x = nextX;
					temp->y = nextY;
					_tree.insert(temp->x + (temp->y * setting.getColumn()));
					myTool::myCout("● ", setting.getTabColumn() + 2 + (temp->x * 2), setting.getTabRow() + 3 + temp->y, 1);
				}
				else {
					while (temp->next->next != nullptr) {
						temp = temp->next;
					}
					_tree.remove(temp->next->x + (temp->next->y * setting.getColumn()));
					myTool::myCout("  ", setting.getTabColumn() + 2 + (temp->next->x * 2), setting.getTabRow() + 3 + temp->next->y);
					myTool::myCout("● ", setting.getTabColumn() + 2 + (_root->x * 2), setting.getTabRow() + 3 + _root->y, 2);
					temp->next->next = _root;
					_root = temp->next;
					temp->next = nullptr;
					_root->x = nextX;
					_root->y = nextY;
					_tree.insert(_root->x + (_root->y * setting.getColumn()));
					myTool::myCout("● ", setting.getTabColumn() + 2 + (_root->x * 2), setting.getTabRow() + 3 + _root->y, 1);
				}
				return true;
			}
		};
		SnakeGame snakeGame;
		void layout() {
			myTool::myCout("貪吃蛇小遊戲，以滑鼠選擇功能", 0, 0, 7);
			myTool::myCout("開始遊戲", 0, 2);
			myTool::myCout("遊戲說明", 0, 4);
			myTool::myCout("設定", 0, 6);
			myTool::myCout("返回上一頁", 0, 8);
		}
		void clearLayout() {
			myTool::clearCmd(0, 0, 27, 8);
		}

		//gameLogic function(gLog) start
		void gLog_layout() {
			myTool::myCout("以WASD控制方向、LShift加減速、Space暫停、Esc退出",
				setting.getTabColumn(), setting.getTabRow(), 6);
			std::string wallBox = "", midWallBox = "■ " + std::string(setting.getColumn() * 2, ' ') + "■";
			for (short i = 0 - 2;i < setting.getColumn();i++) {
				wallBox += "■ ";
			}
			myTool::myCout(wallBox, setting.getTabColumn(), setting.getTabRow() + 2);
			for (short i = 0, tabRow = setting.getTabRow() + 2 + 1;i < setting.getRow();i++) {
				myTool::myCout(midWallBox, setting.getTabColumn(), tabRow + i);
			}
			myTool::myCout(wallBox, setting.getTabColumn(), setting.getTabRow() + 2 + setting.getRow() + 1);
		}
		//gameLogic function(gLog) end

		void gameLogic() {
			setting.init();
			gLog_layout();
			snakeGame.initSnakeGame();
			while (snakeGame.snakeMove());
		}
		void rule() {

		}
		void changeSetting() {

		}
	}
	void start() {
		layout();
		while (true) {
			myTool::resetCursor();
			myTool::mySleep(200);
			myTool::getCursor();
			myTool::colorChangeLayout("開始遊戲", 0, 2, 7, 0, 2);
			myTool::colorChangeLayout("遊戲說明", 0, 4, 7, 0, 2);
			myTool::colorChangeLayout("設定", 0, 6, 3, 0, 2);
			myTool::colorChangeLayout("返回上一頁", 0, 8, 9, 0, 2);
			if (myTool::cursorBox.getState() && myTool::cursorBox.getOneClick() &&
				myTool::cursorBox.getLeftPressed()) {
				if (myTool::cursorTouchArea(0, 2, 7, 0)) {
					clearLayout();
					gameLogic();
					layout();
				}
				else if (myTool::cursorTouchArea(0, 4, 7, 0)) {
					clearLayout();
					rule();
					layout();
				}
				else if (myTool::cursorTouchArea(0, 6, 3, 0)) {
					clearLayout();
					changeSetting();
					layout();
				}
				else if (myTool::cursorTouchArea(0, 8, 9, 0)) {
					clearLayout();
					break;
				}
			}
		}
	}
}