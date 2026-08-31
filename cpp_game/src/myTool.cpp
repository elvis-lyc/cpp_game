#include "myTool.hpp"
#include <iostream>
#include <windows.h>
#include <source_location>
#include <string>
#include <thread>
#include <chrono>
#include <random>

namespace myTool {
	namespace {
		HANDLE getInHandle() {
			static HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
			return hIn;
		}
		HANDLE getOutHandle() {
			static HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			return hOut;
		}
		[[noreturn]] void reportFailure(const std::string& errorText, const std::source_location& loc) {
			std::cerr << "\n##----------##"
				<< "\nError_Info:" + errorText
				<< "\nFunc_Name: " << loc.function_name()
				<< "\nLine: " << loc.line()
				<< "\nColumn: " << loc.column()
				<< "\n##----------##\n";
			std::abort();
		}

		class CursorBitset {
		private:
			Point _position{ 0,0 }, _event{ 0,0 };
			/*
			Bitset
			0~4: _state, _oneClick, _doubleClick, _leftPressed, _rightPressed
			*/
			std::atomic<char> _bit8 = 0;
		public:
			CursorBitset() = default;
			CursorBitset(const CursorBitset& other) {
				_position = other._position;
				_event = other._event;
				_bit8.store(other._bit8.load());
			}

			bool getState() const{
				return _bit8.load() & 1;
			}
			void setState(const bool state) {
				if (state) {
					_bit8.store(_bit8.load() | 1);
				}
				else {
					_bit8.store(_bit8.load() & ~(1));
				}
			}
			bool getOneClick() const{
				return _bit8.load() & 2;
			}
			void setOneClick(const bool oneClk) {
				if (oneClk) {
					_bit8.store(_bit8.load() | 2);
				}
				else {
					_bit8.store(_bit8.load() & ~(2));
				}
			}
			bool getDoubleClick() const{
				return _bit8.load() & 4;
			}
			void setDoubleClick(const bool douClk) {
				if (douClk) {
					_bit8.store(_bit8.load() | 4);
				}
				else {
					_bit8.store(_bit8.load() & ~(4));
				}
			}
			bool getLeftPressed() const{
				return _bit8.load() & 8;
			}
			void setLeftPressed(const bool lPre) {
				if (lPre) {
					_bit8.store(_bit8.load() | 8);
				}
				else {
					_bit8.store(_bit8.load() & ~(8));
				}
			}
			bool getRightPressed() const{
				return _bit8.load() & 16;
			}
			void setRightPressed(const bool rPre) {
				if (rPre) {
					_bit8.store(_bit8.load() | 16);
				}
				else {
					_bit8.store(_bit8.load() & ~(16));
				}
			}

			short getPositionX() const{
				return _position.x;
			}

			short getPositionY() const{
				return _position.y;
			}

			short getEventX() const{
				return _event.x;
			}

			short getEventY() const{
				return _event.y;
			}
			void setPosition(const Point point) {
				myAssert(0 <= point.x && 0 <= point.y,
					"輸入值錯誤 point.x: " + std::to_string(point.x) + ", point.y: " + std::to_string(point.y));
				_position.x = point.x;
				_position.y = point.y;
			}
			void setEvent(const Point point) {
				myAssert(0 <= point.x && 0 <= point.y,
					"輸入值錯誤 point.x: " + std::to_string(point.x) + ", point.y: " + std::to_string(point.y));
				_event.x = point.x;
				_event.y = point.y;
			}
			
			void reset() {
				_bit8.store(0);
			}
		};
		class KeyInputBitset {
		private:
			std::atomic<short> _bit16 = 0;
		public:
			KeyInputBitset() = default;
			KeyInputBitset(const KeyInputBitset& other) {
				_bit16.store(other._bit16.load());
			}

			bool get(const int vKey) const {
				short bit16 = _bit16.load();
				switch (vKey) {
				case 0x1B:
					return bit16 & 1;
				case 0x20:
					return bit16 & 2;
				case 0xA0:
					return bit16 & 4;
				case 0x26:
					return bit16 & 8;
				case 0x27:
					return bit16 & 16;
				case 0x28:
					return bit16 & 32;
				case 0x25:
					return bit16 & 64;
				case 'W':
					return bit16 & 128;
				case 'D':
					return bit16 & 256;
				case 'S':
					return bit16 & 512;
				case 'A':
					return bit16 & 1024;
				default:
					return false;
				}
			}
			/*
			0~10 Esc, Space, LShift, Up, Right, Down, Left, W, D, S, A
			*/
			void set(const short index) {
				myAssert(0 <= index || index <= 10,
					"輸入值錯誤 index: " + std::to_string(index));
				_bit16.store(_bit16.load() | (1 << index));
			}
			void reset() {
				_bit16.store(0);
			}
		};
		CursorBitset csrBit;
		KeyInputBitset keyInpBit;
		std::atomic<bool>isCursor{ false }, isKeyInput{ false };
		std::thread CursorWorker, KeyInputWorker;

		void cursorLoop() {
			if (isCursor.load()) {
				HANDLE hIn = getInHandle();
				DWORD mode, oldMode, eventNum, recordNum, i;
				INPUT_RECORD irBox[16];
				MOUSE_EVENT_RECORD mer;
				GetConsoleMode(hIn, &mode);
				oldMode = mode;
				SetConsoleMode(hIn, (mode & ~ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT));
				auto checkClickCount = [&mer]() {
					if (mer.dwEventFlags == 0) {
						csrBit.setOneClick(true);
						csrBit.setDoubleClick(false);
					}
					else if (mer.dwEventFlags == DOUBLE_CLICK) {
						csrBit.setOneClick(false);
						csrBit.setDoubleClick(true);
					}
				};
				while (isCursor.load()) {
					if (GetNumberOfConsoleInputEvents(hIn, &eventNum) && eventNum != 0) {
						ReadConsoleInputW(hIn, irBox, 16, &recordNum);
						for (i = 0;i < recordNum;i++) {
							if (irBox[i].EventType == MOUSE_EVENT) {
								mer = irBox[i].Event.MouseEvent;
								csrBit.setPosition({ mer.dwMousePosition.X, mer.dwMousePosition.Y });
								if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED) {
									csrBit.setState(true);
									csrBit.setEvent({ mer.dwMousePosition.X, mer.dwMousePosition.Y });
									csrBit.setLeftPressed(true);
									csrBit.setRightPressed(false);
									checkClickCount();
								}
								else if (mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED) {
									csrBit.setState(true);
									csrBit.setEvent({ mer.dwMousePosition.X, mer.dwMousePosition.Y });
									csrBit.setLeftPressed(false);
									csrBit.setRightPressed(true);
									checkClickCount();
								}
							}
						}
					}
					mySleep(10);
				}
				SetConsoleMode(hIn, oldMode);
			}
		}
		void keyInputLoop() {
			while (isKeyInput.load()) {
				if (GetAsyncKeyState(0x1B) & 0x8000) {
					keyInpBit.set(0);
				}
				if (GetAsyncKeyState(0x20) & 0x8000) {
					keyInpBit.set(1);
				}
				if (GetAsyncKeyState(0xA0) & 0x8000) {
					keyInpBit.set(2);
				}
				if (GetAsyncKeyState(0x26) & 0x8000) {
					keyInpBit.set(3);
				}
				else if (GetAsyncKeyState(0x27) & 0x8000) {
					keyInpBit.set(4);
				}
				else if (GetAsyncKeyState(0x28) & 0x8000) {
					keyInpBit.set(5);
				}
				else if (GetAsyncKeyState(0x25) & 0x8000) {
					keyInpBit.set(6);
				}
				if (GetAsyncKeyState('W') & 0x8000) {
					keyInpBit.set(7);
				}
				else if (GetAsyncKeyState('D') & 0x8000) {
					keyInpBit.set(8);
				}
				else if (GetAsyncKeyState('S') & 0x8000) {
					keyInpBit.set(9);
				}
				else if (GetAsyncKeyState('A') & 0x8000) {
					keyInpBit.set(10);
				}
			}
		}
	}

	void Tree::insert(const short data) {
		if (_root == nullptr) {
			_root = new Node(data);
			return;
		}

		Node* temp = _root;
		while (true) {
			if (data < temp->data) {
				if (temp->left != nullptr) {
					temp = temp->left;
				}
				else {
					temp->left = new Node(data);
					return;
				}
			}
			else if (temp->data < data) {
				if (temp->right != nullptr) {
					temp = temp->right;
				}
				else {
					temp->right = new Node(data);
					return;
				}
			}
			else if (data == temp->data) {
				return;
			}
		}
	}
	void Tree::remove(const short data) {
		if (_root == nullptr) {
			return;
		}
		
		Node* temp = _root,* node = nullptr;
		while (true) {
			if (data < temp->data) {
				if (temp->left != nullptr) {
					if (temp->left->left != nullptr ||
						temp->left->right != nullptr) {
						temp = temp->left;
					}
					else {
						if (temp->left->data == data) {
							delete(temp->left);
							temp->left = nullptr;
						}
						else {
							return;
						}
					}
				}
				else {
					return;
				}
			}
			else if (temp->data < data) {
				if (temp->right != nullptr) {
					if (temp->right->left != nullptr ||
						temp->right->right != nullptr) {
						temp = temp->right;
					}
					else {
						if (temp->right->data == data) {
							delete(temp->right);
							temp->right = nullptr;
						}
						else {
							return;
						}
					}
				}
				else {
					return;
				}
			}
			else if (data == temp->data) {
				node = temp;
				if (temp->left != nullptr) {
					temp = temp->left;
					if (temp->right != nullptr) {
						while (temp->right->right != nullptr) {
							temp = temp->right;
						}
						node->data = temp->right->data;
						Node* temp_temp = temp->right;
						temp->right = temp->right->left;
						delete(temp_temp);
						return;
					}
					else {
						node->data = temp->data;
						node->left = temp->left;
						delete(temp);
						return;
					}
				}
				else if (temp->right != nullptr) {
					temp = temp->right;
					if (temp->left != nullptr) {
						while (temp->left->left != nullptr) {
							temp = temp->left;
						}
						node->data = temp->left->data;
						Node* temp_temp = temp->left;
						temp->left = temp->left->right;
						delete(temp_temp);
						return;
					}
					else {
						node->data = temp->data;
						node->right = temp->right;
						delete(temp);
						return;
					}
				}
			}
		}
	}
	void Tree::deleteTree_temp(Tree::Node* node) {
		if (node != nullptr) {
			deleteTree_temp(node->left);
			deleteTree_temp(node->right);
			delete(node);
		}
	}
	void Tree::deleteTree() {
		if (_root != nullptr) {
			deleteTree_temp(_root->left);
			deleteTree_temp(_root->right);
			delete(_root);
			_root = nullptr;
		}
	}
	void Tree::findCanUseIndex_temp(Tree::Node* node, short& index) {
		if(node != nullptr) {
			findCanUseIndex_temp(node->left, index);
			if (node->data <= index) {
				index++;
			}
			findCanUseIndex_temp(node->right, index);
		}
	}
	void Tree::findCanUseIndex(short& index) {
		if (_root != nullptr) {
			findCanUseIndex_temp(_root->left, index);
			if (_root->data <= index) {
				index++;
			}
			findCanUseIndex_temp(_root->right, index);
		}
	}

	bool CursorBox::getState() const {
		return _bit8 & 1;
	}
	bool CursorBox::getOneClick() const {
		return _bit8 & 2;
	}
	bool CursorBox::getDoubleClick() const {
		return _bit8 & 4;
	}
	bool CursorBox::getLeftPressed() const {
		return _bit8 & 8;
	}
	bool CursorBox::getRightPressed() const {
		return _bit8 & 16;
	}
	void CursorBox::set(const bool state, const bool oneClk, const bool douClk,
		const bool lPre, const bool rPre) {
		if (state) {
			_bit8 |= 1;
		}
		else {
			_bit8 &= ~(1);
		}
		if (oneClk) {
			_bit8 |= 2;
		}
		else {
			_bit8 &= ~(2);
		}
		if (douClk) {
			_bit8 |= 4;
		}
		else {
			_bit8 &= ~(4);
		}
		if (lPre) {
			_bit8 |= 8;
		}
		else {
			_bit8 &= ~(8);
		}
		if (rPre) {
			_bit8 |= 16;
		}
		else {
			_bit8 &= ~(16);
		}
	}
	short CursorBox::getPositionX() const {
		return _position.x;
	}
	short CursorBox::getPositionY() const {
		return _position.y;
	}
	short CursorBox::getEventX() const {
		return _event.x;
	}
	short CursorBox::getEventY() const {
		return _event.y;
	}
	void CursorBox::setPosition(const Point point) {
		myAssert(0 <= point.x && 0 <= point.y,
			"輸入值錯誤 point.x: " + std::to_string(point.x) + ", point.y: " + std::to_string(point.y));
		_position.x = point.x;
		_position.y = point.y;
	}
	void CursorBox::setEvent(const Point point) {
		myAssert(0 <= point.x && 0 <= point.y,
			"輸入值錯誤 point.x: " + std::to_string(point.x) + ", point.y: " + std::to_string(point.y));
		_event.x = point.x;
		_event.y = point.y;
	}

	void myAssert(const bool condition, const std::string& errorText, const std::source_location& loc) {
		if (!condition) [[unlikely]] {
			reportFailure(errorText, loc);
		}
	}
	void setCursorXY(const Point point) {
		myAssert(0 <= point.x && 0 <= point.y,
			"輸入值錯誤 point.x: " + std::to_string(point.x) + ", point.y: " + std::to_string(point.y));
		SetConsoleCursorPosition(getOutHandle(), { point.x, point.y });
	}
	void setCursorColor(const short colorIndex) {
		myAssert(0 <= colorIndex && colorIndex <= 255,
			"輸入值錯誤 colorIndex: " + std::to_string(colorIndex));
		SetConsoleTextAttribute(getOutHandle(), colorIndex);
	}
	void setCursorXYC(const Point point, const short colorIndex) {
		setCursorXY({ point.x, point.y });
		setCursorColor(colorIndex);
	}
	void myCout(const std::string& text) {
		std::cout << text;
	}
	void myCout(const std::string& text, const Point point) {
		setCursorXY({ point.x, point.y });
		myCout(text);
	}
	void myCout(const std::string& text, const Point point, const short colorIndex) {
		setCursorXYC({ point.x, point.y }, colorIndex);
		myCout(text);
	}
	bool cursorAtArea(const Point point, const Point countPoint) {
		myAssert(0 <= point.x && 0 <= point.y && 0 <= countPoint.x && 0 <= countPoint.y,
			"輸入值錯誤 point.x: " + std::to_string(point.x) + ", point.y: " + std::to_string(point.y) +
			"\n輸入值錯誤 countPoint.x: " + std::to_string(countPoint.x) + ", countPoint.y: " + std::to_string(countPoint.y));
		if (point.x <= cursorBox.getPositionX() && cursorBox.getPositionX() <= point.x + countPoint.x &&
			point.y <= cursorBox.getPositionY() && cursorBox.getPositionY() <= point.y + countPoint.y) {
			return true;
		}
		return false;
	}
	bool cursorTouchArea(const Point point, const Point countPoint) {
		myAssert(0 <= point.x && 0 <= point.y && 0 <= countPoint.x && 0 <= countPoint.y,
			"輸入值錯誤 point.x: " + std::to_string(point.x) + ", point.y: " + std::to_string(point.y) +
			"\n輸入值錯誤 countPoint.x: " + std::to_string(countPoint.x) + ", countPoint.y: " + std::to_string(countPoint.y));
		if (point.x <= cursorBox.getEventX() && cursorBox.getEventX() <= point.x + countPoint.x &&
			point.y <= cursorBox.getEventY() && cursorBox.getEventY() <= point.y + countPoint.y) {
			return true;
		}
		return false;
	}
	void colorChangeLayout(const std::string& name,
		const Point point, const Point countPoint,
		const short changeColor) {
		myAssert(0 <= point.x && 0 <= point.y && 0 <= countPoint.x && 0 <= countPoint.y && 0 <= changeColor,
			"輸入值錯誤 point.x: " + std::to_string(point.x) + ", point.y: " + std::to_string(point.y) +
			"\n輸入值錯誤 countPoint.x: " + std::to_string(countPoint.x) + ", countPoint.y: " + std::to_string(countPoint.y) +
			"\n輸入值錯誤 changeColor: " + std::to_string(changeColor));
		if (myTool::cursorAtArea({ point.x, point.y }, { countPoint.x, countPoint.y })) {
			myTool::myCout(name, { point.x, point.y }, changeColor);
		}
		else {
			myTool::myCout(name, { point.x, point.y }, 7);
		}
	}
	void mySleep(const int ms) {
		myAssert(0 <= ms,
			"輸入值錯誤 ms: " + std::to_string(ms));
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}
	int myRand(const int firstNum, const int endNum) {
		static std::random_device rd;
		static std::mt19937 gen(rd());
		myAssert(firstNum <= endNum,
			"輸入值錯誤 firstNum: " + std::to_string(firstNum) + ", endNum: " + std::to_string(endNum));
		std::uniform_int_distribution distrib(firstNum, endNum);
		return distrib(gen);
	}
	void clearCmd(const Point point, const Point countPoint, const bool firstOrEndUse) {
		if (firstOrEndUse) {
			system("cls");
		}
		else {
			myAssert(0 <= point.x && 0 <= point.y && 0 <= countPoint.x && 0 <= countPoint.y,
				"輸入值錯誤 point.x: " + std::to_string(point.x) + ", point.y: " + std::to_string(point.y) +
				"\n輸入值錯誤 countPoint.x: " + std::to_string(countPoint.x) + ", countPoint.y: " + std::to_string(countPoint.y));
			std::string spaceText = std::string(1 + countPoint.x, ' ');
			for (short i = 0;i <= countPoint.y;i++) {
				myCout(spaceText, { point.x, safe_cast<short>(point.y + i )});
			}
		}
	}
	void setCmdTitle(const std::wstring& name) {
		SetConsoleTitle(name.c_str());
	}
	void setCursorState(const bool canSee) {
		HANDLE hOut = getOutHandle();
		CONSOLE_CURSOR_INFO cci;
		GetConsoleCursorInfo(hOut, &cci);
		cci.bVisible = canSee;
		SetConsoleCursorInfo(hOut, &cci);
	}

	void startCursor() {
		endCursor();
		isCursor.store(true);
		CursorWorker = std::thread(cursorLoop);
	}
	void endCursor() {
		if (CursorWorker.joinable()) {
			isCursor.store(false);
			CursorWorker.join();
		}
	}
	void getCursor() {
		CursorBitset cb(csrBit);
		cursorBox.set(cb.getState(), cb.getOneClick(), cb.getDoubleClick(),
			cb.getLeftPressed(), cb.getRightPressed());
		cursorBox.setPosition({ cb.getPositionX(), cb.getPositionY() });
		cursorBox.setEvent({ cb.getEventX(), cb.getEventY() });
	}
	void resetCursor() {
		csrBit.reset();
	}
	void startKeyInput() {
		endKeyInput();
		isKeyInput.store(true);
		KeyInputWorker = std::thread(keyInputLoop);
	}
	void endKeyInput() {
		if (KeyInputWorker.joinable()) {
			isKeyInput.store(false);
			KeyInputWorker.join();
		}
	}
	bool getKeyInput(const int vKey) {
		return keyInpBit.get(vKey);
	}
	void resetKeyInput() {
		keyInpBit.reset();
	}
}