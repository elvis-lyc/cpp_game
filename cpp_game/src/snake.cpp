#include "myTool.hpp"
#include "snake.hpp"
#include <string>

namespace snake {
	namespace {
		using myTool::Point;
		using myTool::safe_cast;

		class Setting {
		private:
			/*
			遊戲設定
			*/
			short _column = 10, _row = 10, _moveWaitTime = 500,
				_LShiftWaitTime = 250, _foodNum = 2,
				_score = 0, _mapArea = _row * _column, _maxScore = _mapArea - 1;
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
			short getMoveWaitTime() const{
				return _moveWaitTime;
			}
			short getLShiftWaitTime() const{
				return _LShiftWaitTime;
			}
			short getFoodNum() const{
				return _foodNum;
			}
			short getScore() const{
				return _score;
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
				myTool::myAssert(_minColumn <= column && column <= _maxColumn,
					"輸入值錯誤 column: " + std::to_string(column));
				_column = column;
			}
			void setRow(const short row) {
				myTool::myAssert(_minRow <= row && row <= _maxRow,
					"輸入值錯誤 row: " + std::to_string(row));
				_row = row;
			}
			void setMoveWaitTime(const short moveWaitTime) {
				myTool::myAssert(_minWaitTime <= moveWaitTime && moveWaitTime <= _maxWaitTime,
					"輸入值錯誤 moveWaitTime: " + std::to_string(moveWaitTime));
				_moveWaitTime = moveWaitTime;
			}
			void setLShiftWaitTime(const short LShiftWaitTime) {
				myTool::myAssert(_minWaitTime <= LShiftWaitTime && LShiftWaitTime <= _maxWaitTime,
					"輸入值錯誤 LShiftWaitTime: " + std::to_string(LShiftWaitTime));
				_LShiftWaitTime = LShiftWaitTime;
			}
			void setFoodNum(const short foodNum) {
				myTool::myAssert(_minFoodNum <= foodNum && foodNum <= _maxFoodNum,
					"輸入值錯誤 foodNum: " + std::to_string(foodNum));
				_foodNum = foodNum;
			}
			void setScore(const short score) {
				myTool::myAssert(score <= _maxScore,
					"輸入值錯誤 score: " + std::to_string(score));
				_score = score;
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
				Point point;
				Node* next = nullptr;
				Node() = default;
				Node(Point input) :point{ input } {}
				void setXY(const Point input) {
					point.x = input.x;
					point.y = input.y;
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
			short _foodBoxNum = 0, _remainingIndex = 0;
			Point _move;

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
				_move = { 0,-1 };
			}
		public:
			void initSnakeGame(const Point point = { safe_cast<short>(setting.getColumn() / 2), safe_cast<short>(setting.getRow() / 2)},
				const short num = (setting.getMaxScore() < setting.getFoodNum()) ? setting.getMaxScore() : setting.getFoodNum()) {
				deleteSnakeGame();
				/*
				以輸入的(x,y)建立與顯示頭部位置
				{預設置中(或偏右下)}
				*/
				myTool::myAssert(0 <= point.x && point.x < setting.getColumn() &&
					0 <= point.y && point.y < setting.getRow(),
					"輸入值錯誤 point.x: " + std::to_string(point.x) + ", point.y: " + std::to_string(point.y));
				_remainingIndex = setting.getMaxScore();
				_root = new Node(point);
				myTool::myCout("● ", {
					safe_cast<short>(setting.getTabColumn() + 2 + (point.x * 2)),
					safe_cast<short>(setting.getTabRow() + 3 + point.y)
					}, 1);
				_tree.insert(point.x + (point.y * setting.getColumn()));
				_remainingIndex--;

				/*
				以輸入的(num)選擇要初始化與隨機位置的果子數量
				*/
				myTool::myAssert(num <= 10,
					"輸入值錯誤 num: " + std::to_string(num));
				_foodBoxNum = num;
				Point tempPoint;
				for (short i = 0, randIndex = 0;i < _foodBoxNum;i++) {
					randIndex = myTool::myRand(0, _remainingIndex);
					_tree.findCanUseIndex(randIndex);
					tempPoint = { safe_cast<short>(randIndex % setting.getColumn()), safe_cast<short>(randIndex / setting.getColumn()) };
					_foodBox[i] = new Node(tempPoint);
					myTool::myCout("● ", {
						safe_cast<short>(setting.getTabColumn() + 2 + (tempPoint.x * 2)),
						safe_cast<short>(setting.getTabRow() + 3 + tempPoint.y)
						}, 4);
					_tree.insert(randIndex);
					_remainingIndex--;
				}

				myTool::resetKeyInput();
			}

			bool willGameOver(const short num) const{
				if (num == 0) {
					return false;
				}

				short mapColumn = 3 + (setting.getColumn() * 2), mapRow = 4 + setting.getRow();
				myTool::clearCmd({ setting.getTabColumn(), setting.getTabRow() }, { safe_cast<short>((47 < mapColumn) ? mapColumn : 47), mapRow });
				switch (num) {
				case 1:
					myTool::myCout("以Esc退出\n\n最後得分: " +
						std::to_string(setting.getScore()) +
						"\n\n按空白鍵返回..", { 0,0 }, 7);
					while (true) {
						myTool::resetKeyInput();
						myTool::mySleep(200);
						if (myTool::getKeyInput(0x20)) {
							break;
						}
					}
					myTool::clearCmd({ 0,0 }, { 13,4 });
					return true;
					break;
				case 2:
					myTool::myCout("蛇撞牆\n\n最後得分: " +
						std::to_string(setting.getScore()) +
						"\n\n按空白鍵返回..", { 0,0 }, 7);
					while (true) {
						myTool::resetKeyInput();
						myTool::mySleep(200);
						if (myTool::getKeyInput(0x20)) {
							break;
						}
					}
					myTool::clearCmd({ 0,0 }, { 13,4 });
					return true;
					break;
				case 3:
					myTool::myCout("蛇咬到自己\n\n最後得分: " +
						std::to_string(setting.getScore()) +
						"\n\n按空白鍵返回..", { 0,0 }, 7);
					while (true) {
						myTool::resetKeyInput();
						myTool::mySleep(200);
						if (myTool::getKeyInput(0x20)) {
							break;
						}
					}
					myTool::clearCmd({ 0,0 }, { 13,4 });
					return true;
					break;
				case 4:
					myTool::myCout("蛇佔滿整個地圖，遊戲勝利\n\n最後得分: " +
						std::to_string(setting.getScore()) +
						"\n\n按空白鍵返回..", { 0,0 }, 7);
					while (true) {
						myTool::resetKeyInput();
						myTool::mySleep(200);
						if (myTool::getKeyInput(0x20)) {
							break;
						}
					}
					myTool::clearCmd({ 0,0 }, { 14,4 });
					return true;
					break;
				}
				myTool::myAssert(false, "willGameOver(num) num錯誤: " + std::to_string(num));
			}
			short willSnakeDie(const Point nextPoint) const{
				if ((nextPoint.x < 0 || setting.getColumn() <= nextPoint.x)||
					(nextPoint.y < 0 || setting.getRow() <= nextPoint.y)) {
					return 2;
				}
				
				Node* temp = _root;
				while (temp->next != nullptr) {
					if (temp->point.x == nextPoint.x && temp->point.y == nextPoint.y) {
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
				if (myTool::getKeyInput(0x20)) {
					myTool::mySleep(200);
					while (true) {
						myTool::resetKeyInput();
						myTool::mySleep(200);
						if (myTool::getKeyInput(0x20)) {
							myTool::mySleep(200);
							break;
						}
					}
				}
				myTool::resetKeyInput();
				myTool::mySleep(waitTime);
				if (myTool::getKeyInput(0x1B)) {
					willGameOver(1);
					return false;
				}
				if (useWASD) {
					if (myTool::getKeyInput('W') && _move.y != 1) {
						_move = { 0,-1 };
					}
					else if (myTool::getKeyInput('D') && _move.x != -1) {
						_move = { 1,0 };
					}
					else if (myTool::getKeyInput('S') && _move.y != -1) {
						_move = { 0,1 };
					}
					else if (myTool::getKeyInput('A') && _move.x != 1) {
						_move = { -1,0 };
					}
				}
				else {
					if (myTool::getKeyInput(0x26) && _move.y != 1) {
						_move = { 0,-1 };
					}
					else if (myTool::getKeyInput(0x27) && _move.x != -1) {
						_move = { 1,0 };
					}
					else if (myTool::getKeyInput(0x28) && _move.y != -1) {
						_move = { 0,1 };
					}
					else if (myTool::getKeyInput(0x25) && _move.x != 1) {
						_move = { -1,0 };
					}
				}

				Point nextPoint = { safe_cast<short>(_root->point.x + _move.x), safe_cast<short>(_root->point.y + _move.y) };
				if (willGameOver(willSnakeDie(nextPoint))) {
					return false;
				}

				for (short i = 0;i < _foodBoxNum;i++) {
					if (_foodBox[i] == nullptr) {
						continue;
					}
					if (_foodBox[i]->point.x == nextPoint.x && _foodBox[i]->point.y == nextPoint.y) {
						_foodBox[i]->next = _root;
						_root = _foodBox[i];
						_foodBox[i] = nullptr;
						myTool::myCout("● ",{
							safe_cast<short>(setting.getTabColumn() + 2 + (_root->point.x * 2)),
							safe_cast<short>(setting.getTabRow() + 3 + _root->point.y)
							}, 1);
						myTool::myCout("● ", {
							safe_cast<short>(setting.getTabColumn() + 2 + (_root->next->point.x * 2)),
							safe_cast<short>(setting.getTabRow() + 3 + _root->next->point.y)
							}, 2);

						if (0 <= _remainingIndex) {
							Point tempPoint;
							short randIndex = myTool::myRand(0, _remainingIndex);
							_tree.findCanUseIndex(randIndex);
							tempPoint = { safe_cast<short>(randIndex % setting.getColumn()),  safe_cast<short>(randIndex / setting.getColumn()) };
							_foodBox[i] = new Node(tempPoint);
							myTool::myCout("● ", {
								safe_cast<short>(setting.getTabColumn() + 2 + (tempPoint.x * 2)),
								safe_cast<short>(setting.getTabRow() + 3 + tempPoint.y)
								}, 4);
							_tree.insert(randIndex);
							_remainingIndex--;
						}
						setting.setScore(setting.getScore() + 1);
						return true;
					}
				}

				Node* temp = _root;
				if (temp->next == nullptr) {
					_tree.remove(temp->point.x + (temp->point.y * setting.getColumn()));
					myTool::myCout("  ", {
						safe_cast<short>(setting.getTabColumn() + 2 + (temp->point.x * 2)),
						safe_cast<short>(setting.getTabRow() + 3 + temp->point.y) });
					temp->point = { nextPoint };
					_tree.insert(temp->point.x + (temp->point.y * setting.getColumn()));
					myTool::myCout("● ", {
						safe_cast<short>(setting.getTabColumn() + 2 + (temp->point.x * 2)),
						safe_cast<short>(setting.getTabRow() + 3 + temp->point.y)
						}, 1);
				}
				else {
					while (temp->next->next != nullptr) {
						temp = temp->next;
					}
					_tree.remove(temp->next->point.x + (temp->next->point.y * setting.getColumn()));
					myTool::myCout("  ", {
						safe_cast<short>(setting.getTabColumn() + 2 + (temp->next->point.x * 2)),
						safe_cast<short>(setting.getTabRow() + 3 + temp->next->point.y) });
					myTool::myCout("● ", {
						safe_cast<short>(setting.getTabColumn() + 2 + (_root->point.x * 2)),
						safe_cast<short>(setting.getTabRow() + 3 + _root->point.y)
						}, 2);
					temp->next->next = _root;
					_root = temp->next;
					temp->next = nullptr;
					_root->point = { nextPoint };
					_tree.insert(_root->point.x + (_root->point.y * setting.getColumn()));
					myTool::myCout("● ", {
						safe_cast<short>(setting.getTabColumn() + 2 + (_root->point.x * 2)),
						safe_cast<short>(setting.getTabRow() + 3 + _root->point.y)
						}, 1);
				}
				return true;
			}
		};
		SnakeGame snakeGame;
		void layout() {
			myTool::myCout("貪吃蛇小遊戲，以滑鼠選擇功能", { 0,0 }, 7);
			myTool::myCout("開始遊戲", { 0,2 });
			myTool::myCout("遊戲說明", { 0,4 });
			myTool::myCout("設定", { 0,6 });
			myTool::myCout("返回上一頁", { 0,8 });
		}
		void clearLayout() {
			myTool::clearCmd({ 0,0 }, { 27,8 });
		}

		//gameLogic function(gLog) start
		void gLog_layout() {
			myTool::myCout("以WASD控制方向、LShift加減速、Space暫停、Esc退出",
				{ setting.getTabColumn(), setting.getTabRow() }, 6);
			std::string wallBox = "", midWallBox = "■ " + std::string(setting.getColumn() * 2, ' ') + "■";
			for (short i = 0 - 2;i < setting.getColumn();i++) {
				wallBox += "■ ";
			}
			myTool::myCout(wallBox, { setting.getTabColumn(), safe_cast<short>(setting.getTabRow() + 2) });
			for (short i = 0, tabRow = setting.getTabRow() + 2 + 1;i < setting.getRow();i++) {
				myTool::myCout(midWallBox, { setting.getTabColumn(), safe_cast<short>(tabRow + i) });
			}
			myTool::myCout(wallBox, { setting.getTabColumn(), safe_cast<short>(setting.getTabRow() + 2 + setting.getRow() + 1) });
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
			myTool::colorChangeLayout("開始遊戲", { 0,2 }, { 7,0 }, 2);
			myTool::colorChangeLayout("遊戲說明", { 0,4 }, { 7,0 }, 2);
			myTool::colorChangeLayout("設定", { 0,6 }, { 3,0 }, 2);
			myTool::colorChangeLayout("返回上一頁", { 0,8 }, { 9,0 }, 2);
			if (myTool::cursorBox.getState() && myTool::cursorBox.getOneClick() &&
				myTool::cursorBox.getLeftPressed()) {
				if (myTool::cursorTouchArea({ 0,2 }, { 7,0 })) {
					clearLayout();
					gameLogic();
					layout();
				}
				else if (myTool::cursorTouchArea({ 0,4 }, { 7,0 })) {
					clearLayout();
					rule();
					layout();
				}
				else if (myTool::cursorTouchArea({ 0,6 }, { 3,0 })) {
					clearLayout();
					changeSetting();
					layout();
				}
				else if (myTool::cursorTouchArea({ 0,8 }, { 9,0 })) {
					clearLayout();
					break;
				}
			}
		}
	}
}