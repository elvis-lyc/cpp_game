#include "myTool.hpp"
#include "snake.hpp"
#include <string>
#include <vector>

namespace snake {
	namespace {
		using myTool::Tree;
		using myTool::Point;
		using myTool::safe_cast;

		class Setting {
		private:
			/*
			遊戲設定:
			地圖範圍(寬 和 高): _column, _row
			停頓時間(正常 與 按住LShift): _moveWaitTime, _LShiftWaitTime 
			應同時存在多少食物: _foodCount
			地圖總格數 與 最大得分數:_mapArea = _row * _column, _maxScore = _mapArea - 1
			操作方式(WASD(true) 或 方向鍵(false)): _useWASD
			遊戲平移格數(水平 和 垂直): _tabColumn, _tabRow
			*/
			short _column = 10, _row = 10, _moveWaitTime = 500,
				_LShiftWaitTime = 250, _foodCount = 2,
				_mapArea = _row * _column, _maxScore = _mapArea - 1,
				_tabColumn = 0, _tabRow = 0;
			bool _useWASD = true;
			/*
			上下限定義、常數:
			邏輯依 遊戲設定 類推
			*/
			const short _minColumn = 1, _maxColumn = 30, _minRow = 1, _maxRow = 30,
				_minWaitTime = 200, _maxWaitTime = 1000, _minFoodCount = 1, _maxFoodCount = 10;
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
			short getFoodCount() const{
				return _foodCount;
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
			void setFoodCount(const short foodCount) {
				myTool::myAssert(_minFoodCount <= foodCount && foodCount <= _maxFoodCount,
					"輸入值錯誤 foodCount: " + std::to_string(foodCount));
				_foodCount = foodCount;
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
			
			/*
			依遊戲設定來 顯示畫面 與 提示詞
			*/
			void layout() const{
				myTool::myCout("以WASD控制方向、LShift加減速、Space暫停、Esc退出",
					{ getTabColumn(), getTabRow() }, 6);
				std::string wallBox = "", midWallBox = "■ " + std::string(getColumn() * 2, ' ') + "■";
				for (short i = 0 - 2;i < getColumn();i++) {
					wallBox += "■ ";
				}
				myTool::myCout(wallBox, { getTabColumn(), safe_cast<short>(getTabRow() + 2) });
				for (short i = 0, tabRow = getTabRow() + 2 + 1;i < getRow();i++) {
					myTool::myCout(midWallBox, { getTabColumn(), safe_cast<short>(tabRow + i) });
				}
				myTool::myCout(wallBox, { getTabColumn(), safe_cast<short>(getTabRow() + 2 + getRow() + 1) });
				myTool::myCout("目前遊戲狀態: 進行中", {
					safe_cast<short>(getTabColumn() + 4 + (getColumn() * 2)),
					safe_cast<short>(getTabRow() + 2) });
				myTool::myCout("目前分數: 0", {
					safe_cast<short>(getTabColumn() + 4 + (getColumn() * 2)),
					safe_cast<short>(getTabRow() + 3) });
			}
		};

		class SnakeGame {
		private:
			/*
			蛇節點與食物共用結構
			*/
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
			蛇頭指標
			*/
			Node* _root = nullptr;

			/*
			食物位置容器
			*/
			std::vector<Node*> _foodBox;

			/*
			_score紀錄當前得分
			*/
			short _score = 0;

			/*
			移動方向:
			_move.x(-1, 1): (上, 下)
			_move.y(-1, 1): (左, 右)
			例: _move{0, -1}(上)
			*/
			Point _move;

			/*
			初始化所需資源
			*/
			void initSnakeGame(Setting& setting, Tree& tree, short& remainingIndex) {
				/*
				初始化:
				地圖與提示詞;
				蛇移動方向(向上);
				分數(歸0);
				*/
				setting.layout();
				_move = { 0,-1 };
				_score = 0;

				/*
				初始化蛇頭位置(head);
				檢查 head 是否在地圖範圍內;
				建立蛇頭結構;
				並在 cmd 上繪出;
				*/
				Point head = { safe_cast<short>(setting.getColumn() / 2), safe_cast<short>(setting.getRow() / 2) };
				myTool::myAssert(0 <= head.x && head.x < setting.getColumn() &&
					0 <= head.y && head.y < setting.getRow(),
					"輸入值錯誤 head.x: " + std::to_string(head.x) + ", head.y: " + std::to_string(head.y));
				_root = new Node(head);
				myTool::myCout("● ", {
					safe_cast<short>(setting.getTabColumn() + 2 + (head.x * 2)),
					safe_cast<short>(setting.getTabRow() + 3 + head.y)
					}, 1);

				/*
				初始化剩餘可使用格子數(與Tree配合使用);
				將蛇頭所在位置的索引值加入到Tree;
				剩餘可使用格子數 減1;
				*/
				remainingIndex = setting.getMaxScore();
				tree.insert(head.x + (head.y * setting.getColumn()));
				remainingIndex--;

				/*
				初始化 最大可生成的食物數(maxFoodCount) 並確保其不會大於 地圖剩餘可使用格子數(maxScore);
				*/
				short maxFoodCount = (setting.getMaxScore() < setting.getFoodCount()) ? setting.getMaxScore() : setting.getFoodCount();
				myTool::myAssert(maxFoodCount <= 10,
					"輸入值錯誤 _maxFoodCount: " + std::to_string(maxFoodCount));

				/*
				依照_maxFoodCount數量生成食物
				*/
				Point food;
				short randIndex = 0;
				_foodBox = std::vector<Node*>(maxFoodCount, nullptr);
				for (auto& node : _foodBox) {
					/*
					隨機取一值(索引值);
					確保randIndex索引值是可用的;
					*/
					randIndex = myTool::myRand(0, remainingIndex);
					tree.findCanUseIndex(randIndex);

					/*
					建立食物座標;
					並初始化至 node : _foodBox;
					將食物顯示於 cmd;
					*/
					food = { safe_cast<short>(randIndex % setting.getColumn()), safe_cast<short>(randIndex / setting.getColumn()) };
					node = new Node(food);
					myTool::myCout("● ", {
						safe_cast<short>(setting.getTabColumn() + 2 + (food.x * 2)),
						safe_cast<short>(setting.getTabRow() + 3 + food.y)
						}, 4);

					/*
					將食物所在位置的索引值加入到Tree;;
					剩餘可使用格子數 減1;
					*/
					tree.insert(randIndex);
					remainingIndex--;
				}

				/*
				避免殘留按鍵紀錄導致開局就暫停
				*/
				myTool::resetKeyInput();
			}

			/*
			清除所佔用資源
			*/
			void deleteSnakeGame(Tree& tree) {
				/*
				清除蛇串列所佔資源
				*/
				if (_root != nullptr) {
					Node* temp = nullptr;
					do {
						temp = _root->next;
						delete(_root);
						_root = temp;
					} while (_root != nullptr);
					_root = nullptr;
				}

				/*
				清除Tree所佔資源
				*/
				tree.deleteTree();

				/*
				清除食物容器所佔資源
				*/
				for (auto& node : _foodBox) {
					delete node;
				}
				_foodBox.clear();
			}

			/*
			輸入 num 判斷是否繼續 或 檢查以何種方式結束遊戲
			num:
			0: 無事
			1: 以 Esc 退出
			2: 蛇撞牆
			3: 蛇咬到自己
			4: 遊戲勝利
			*/
			bool willGameOver(const Setting& setting, const short num) const {
				/*
				num = 0，繼續遊戲
				*/
				if (num == 0) {
					return false;
				}

				/*
				地圖大小(寬 和 高): mapColumn, mapRow;
				清除遊戲畫面;
				*/
				short mapColumn = 3 + (setting.getColumn() * 2 + 18), mapRow = 4 + setting.getRow();
				myTool::clearCmd({ setting.getTabColumn(), setting.getTabRow() }, { safe_cast<short>((47 < mapColumn) ? mapColumn : 47), mapRow });

				/*
				依對應的 num 做出反應
				*/
				switch (num) {
				case 1:
					myTool::myCout("以Esc退出\n\n最後得分: " +
						std::to_string(_score) +
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
						std::to_string(_score) +
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
						std::to_string(_score) +
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
						std::to_string(_score) +
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
				/*
				num 應在 0 ~ 4 之間
				*/
				myTool::myAssert(false, "不應到此，willGameOver(num) num錯誤: " + std::to_string(num));
			}

			/*
			檢查蛇是否撞牆、咬到自己
			回傳 0: 正常(遊戲繼續)
			回傳 2: 蛇撞牆(遊戲結束)
			回傳 3: 蛇咬到自己(遊戲結束)
			*/
			short willSnakeDie(const Setting& setting, const Point nextPoint) const {
				/*
				檢查是否撞牆
				*/
				if ((nextPoint.x < 0 || setting.getColumn() <= nextPoint.x) ||
					(nextPoint.y < 0 || setting.getRow() <= nextPoint.y)) {
					return 2;
				}

				/*
				檢查是否咬到自己
				*/
				Node* temp = _root;
				while (temp->next != nullptr) {
					if (temp->point == nextPoint) {
						return 3;
					}
					else {
						temp = temp->next;
					}
				}

				/*
				皆無則返回0
				*/
				return 0;
			}

			/*
			處理鍵盤狀態
			回傳 0: 正常(遊戲繼續)
			回傳 1: 按下 Esc(遊戲結束)
			*/
			short keyInput(const Setting& setting) {
				/*
				停頓時間
				*/
				short waitTime = 0;
				if (myTool::getKeyInput(0xA0)) {
					waitTime = setting.getLShiftWaitTime();
				}
				else {
					waitTime = setting.getMoveWaitTime();
				}

				/*
				是否暫停
				*/
				if (myTool::getKeyInput(0x20)) {
					myTool::myCout("暫停中", {
						safe_cast<short>(setting.getTabColumn() + 4 + (setting.getColumn() * 2) + 14),
						safe_cast<short>(setting.getTabRow() + 2)
						}, 6);
					myTool::mySleep(200);
					while (true) {
						myTool::resetKeyInput();
						myTool::mySleep(200);
						if (myTool::getKeyInput(0x20)) {
							myTool::myCout("進行中", {
								safe_cast<short>(setting.getTabColumn() + 4 + (setting.getColumn() * 2) + 14),
								safe_cast<short>(setting.getTabRow() + 2)
								}, 6);
							myTool::mySleep(200);
							break;
						}
					}
				}

				/*
				清除殘留按鍵紀錄;
				等待對應時間;
				*/
				myTool::resetKeyInput();
				myTool::mySleep(waitTime);

				/*
				是否以 Esc 退出
				*/
				if (myTool::getKeyInput(0x1B)) {
					return 1;
				}

				/*
				移動
				*/
				if (setting.getUseWASD()) {
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
				return 0;
			}

			/*
			處裡蛇移動問題
			回傳 true: 正常(遊戲繼續)
			回傳 false: 結束(遊戲結束)
			*/
			bool snakeMove(const Setting& setting, Tree& tree, short& remainingIndex) {
				/*
				檢查是否分數達到最高分(勝利，遊戲結束)
				*/
				if (setting.getMaxScore() <= _score) {
					willGameOver(setting, 4);
					return false;
				}

				/*
				檢查是否按下 Esc(遊戲結束)
				*/
				if (willGameOver(setting, keyInput(setting))) {
					return false;
				}

				/*
				檢查是否 撞牆、咬到自己(遊戲結束)
				*/
				Point nextPoint = _root->point + _move;
				if (willGameOver(setting, willSnakeDie(setting, nextPoint))) {
					return false;
				}

				/*
				檢查蛇是否有吃到食物
				*/
				for (auto& node : _foodBox) {
					if (node == nullptr) {
						continue;
					}

					/*
					檢查蛇下一步上是否有食物
					並顯示新畫面
					*/
					if (node->point == nextPoint) {
						node->next = _root;
						_root = node;
						node = nullptr;
						myTool::myCout("● ", {
							safe_cast<short>(setting.getTabColumn() + 2 + (_root->point.x * 2)),
							safe_cast<short>(setting.getTabRow() + 3 + _root->point.y)
							}, 1);
						myTool::myCout("● ", {
							safe_cast<short>(setting.getTabColumn() + 2 + (_root->next->point.x * 2)),
							safe_cast<short>(setting.getTabRow() + 3 + _root->next->point.y)
							}, 2);

						/*
						判斷是否還需要建立新食物
						並顯示新畫面
						*/
						if (0 <= remainingIndex) {
							Point tempPoint;
							short randIndex = myTool::myRand(0, remainingIndex);
							tree.findCanUseIndex(randIndex);
							tempPoint = { safe_cast<short>(randIndex % setting.getColumn()),  safe_cast<short>(randIndex / setting.getColumn()) };
							node = new Node(tempPoint);
							myTool::myCout("● ", {
								safe_cast<short>(setting.getTabColumn() + 2 + (tempPoint.x * 2)),
								safe_cast<short>(setting.getTabRow() + 3 + tempPoint.y)
								}, 4);
							tree.insert(randIndex);
							remainingIndex--;
						}
						_score++;
						myTool::myCout(std::to_string(_score), {
							safe_cast<short>(setting.getTabColumn() + 4 + (setting.getColumn() * 2) + 10),
							safe_cast<short>(setting.getTabRow() + 3)
							}, 6);
						return true;
					}
				}

				/*
				未吃到食物(正常移動)
				並顯示新畫面
				*/
				Node* temp = _root;
				if (temp->next == nullptr) {
					tree.remove(temp->point.x + (temp->point.y * setting.getColumn()));
					myTool::myCout("  ", {
						safe_cast<short>(setting.getTabColumn() + 2 + (temp->point.x * 2)),
						safe_cast<short>(setting.getTabRow() + 3 + temp->point.y) });
					temp->point = { nextPoint };
					tree.insert(temp->point.x + (temp->point.y * setting.getColumn()));
					myTool::myCout("● ", {
						safe_cast<short>(setting.getTabColumn() + 2 + (temp->point.x * 2)),
						safe_cast<short>(setting.getTabRow() + 3 + temp->point.y)
						}, 1);
				}
				else {
					while (temp->next->next != nullptr) {
						temp = temp->next;
					}
					tree.remove(temp->next->point.x + (temp->next->point.y * setting.getColumn()));
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
					tree.insert(_root->point.x + (_root->point.y * setting.getColumn()));
					myTool::myCout("● ", {
						safe_cast<short>(setting.getTabColumn() + 2 + (_root->point.x * 2)),
						safe_cast<short>(setting.getTabRow() + 3 + _root->point.y)
						}, 1);
				}
				return true;
			}
		public:
			/*
			外部接口
			*/
			void run(Setting& setting, Tree& tree, short& remainingIndex) {
				/*
				初始化資源;
				遊戲主循環;
				釋放資源;
				*/
				initSnakeGame(setting, tree, remainingIndex);
				while (snakeMove(setting, tree, remainingIndex));
				deleteSnakeGame(tree);
			}
		};
		/*
		遊戲頁面布局
		*/
		void layout() {
			myTool::myCout("貪吃蛇小遊戲，以滑鼠選擇功能", { 0,0 }, 7);
			myTool::myCout("開始遊戲", { 0,2 });
			myTool::myCout("遊戲說明", { 0,4 });
			myTool::myCout("設定", { 0,6 });
			myTool::myCout("返回上一頁", { 0,8 });
		}

		/*
		清除遊戲頁面
		*/
		void clearLayout() {
			myTool::clearCmd({ 0,0 }, { 27,8 });
		}

		/*
		遊戲主邏輯
		*/
		void gameLogic(Setting& setting) {
			/*
			遊戲所需的類
			*/
			SnakeGame snakeGame;

			/*
			{以索引值(下標)儲存資訊}
			儲存已被佔用的地圖格子(0 ~ (setting::getMapArea - 1))
			方便找到未被佔用的地圖格子
			*/
			Tree tree;

			/*
			{以索引值(下標)儲存資訊}
			紀錄剩餘可使用格子數(與Tree配合使用)
			*/
			short remainingIndex = 0;

			/*
			遊戲初始化;
			遊戲主邏輯;
			釋放資源;
			*/
			snakeGame.run(setting, tree, remainingIndex);
		}

		/*
		顯示遊戲規則頁面
		*/
		void rule() {

		}

		/*
		顯示設定頁面
		並處理 setting 更新問題
		*/
		void changeSetting() {

		}
	}
	void start() {
		/*
		遊戲所需設定(以 changeSetting()更改 、 gameLogic()使用 )
		*/
		Setting setting;

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
					gameLogic(setting);
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