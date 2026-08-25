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
		[[noreturn]] void reportFailure(const std::source_location& loc) {
			std::cerr << "\n##----------##"
				<< "\nError_Info:"
				<< "\nFunc_Name: " << loc.function_name()
				<< "\nLine: " << loc.line()
				<< "\nColumn: " << loc.column()
				<< "\n##----------##\n";
			std::abort();
		}

		class CursorBitset {
		private:
			short _positionX = 0, _positionY = 0, _eventX = 0, _eventY = 0;
			/*
			Bitset
			0~4: _state, _oneClick, _doubleClick, _leftPressed, _rightPressed
			*/
			std::atomic<char> _bit8 = 0;
		public:
			CursorBitset() = default;
			CursorBitset(const CursorBitset& other) {
				_positionX = other._positionX;
				_positionY = other._positionY;
				_eventX = other._eventX;
				_eventY = other._eventY;
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
				return _positionX;
			}

			short getPositionY() const{
				return _positionY;
			}

			short getEventX() const{
				return _eventX;
			}

			short getEventY() const{
				return _eventY;
			}
			void setPosition(const short x, const short y) {
				myAssert(0 <= x && 0 <= y);
				_positionX = x;
				_positionY = y;
			}
			void setEvent(const short x, const short y) {
				myAssert(0 <= x && 0 <= y);
				_eventX = x;
				_eventY = y;
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
				myAssert(0 <= index || index <= 10);
				reset();
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
								csrBit.setPosition(mer.dwMousePosition.X, mer.dwMousePosition.Y);
								if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED) {
									csrBit.setState(true);
									csrBit.setEvent(mer.dwMousePosition.X, mer.dwMousePosition.Y);
									csrBit.setLeftPressed(true);
									csrBit.setRightPressed(false);
									checkClickCount();
								}
								else if (mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED) {
									csrBit.setState(true);
									csrBit.setEvent(mer.dwMousePosition.X, mer.dwMousePosition.Y);
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
				else if (GetAsyncKeyState(0x20) & 0x8000) {
					keyInpBit.set(1);
				}
				else if (GetAsyncKeyState(0xA1) & 0x8000) {
					keyInpBit.set(2);
				}
				else if (GetAsyncKeyState(0x26) & 0x8000) {
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
				else if (GetAsyncKeyState('W') & 0x8000) {
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
		return _positionX;
	}
	short CursorBox::getPositionY() const {
		return _positionY;
	}
	short CursorBox::getEventX() const {
		return _eventX;
	}
	short CursorBox::getEventY() const {
		return _eventY;
	}
	void CursorBox::setPosition(const short x, const short y) {
		myAssert(0 <= x && 0 <= y);
		_positionX = x;
		_positionY = y;
	}
	void CursorBox::setEvent(const short x, const short y) {
		myAssert(0 <= x && 0 <= y);
		_eventX = x;
		_eventY = y;
	}

	void myAssert(const bool condition, const std::source_location& loc) {
		if (!condition) [[unlikely]] {
			reportFailure(loc);
		}
	}
	void setCursorXY(const short x, const short y) {
		myAssert(0 <= x && 0 <= y);
		SetConsoleCursorPosition(getOutHandle(), COORD(x, y));
	}
	void setCursorColor(const short colorIndex) {
		myAssert(0 <= colorIndex && colorIndex <= 255);
		SetConsoleTextAttribute(getOutHandle(), colorIndex);
	}
	void setCursorXYC(const short x, const short y, const short colorIndex) {
		setCursorXY(x, y);
		setCursorColor(colorIndex);
	}
	void myCout(const std::string& text) {
		std::cout << text;
	}
	void myCout(const std::string& text, const short x, const short y) {
		setCursorXY(x, y);
		myCout(text);
	}
	void myCout(const std::string& text, const short x, const short y, const short colorIndex) {
		setCursorXYC(x, y, colorIndex);
		myCout(text);
	}
	bool cursorAtArea(const short x, const short y, const short countX, const short countY) {
		myAssert(0 <= x && 0 <= y && 0 <= countX && 0 <= countY);
		if (x <= cursorBox.getPositionX() && cursorBox.getPositionX() <= x + countX &&
			y <= cursorBox.getPositionY() && cursorBox.getPositionY() <= y + countY) {
			return true;
		}
		return false;
	}
	bool cursorTouchArea(const short x, const short y, const short countX, const short countY) {
		myAssert(0 <= x && 0 <= y && 0 <= countX && 0 <= countY);
		if (x <= cursorBox.getEventX() && cursorBox.getEventX() <= x + countX &&
			y <= cursorBox.getEventY() && cursorBox.getEventY() <= y + countY) {
			return true;
		}
		return false;
	}
	void colorChangeLayout(const std::string& name,
		const short x, const short y, const short countX, const short countY,
		const short changeColor) {
		myAssert(0 <= x && 0 <= y && 0 <= countX && 0 <= countY && 0 <= changeColor);
		if (myTool::cursorAtArea(x, y, countX, countY)) {
			myTool::myCout(name, x, y, changeColor);
		}
		else {
			myTool::myCout(name, x, y, 7);
		}
	}
	void mySleep(const int ms) {
		myAssert(0 <= ms);
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}
	int myRand(const int firstNum, const int endNum) {
		static std::random_device rd;
		static std::mt19937 gen(rd());
		myAssert(firstNum <= endNum);
		std::uniform_int_distribution distrib(firstNum, endNum);
		return distrib(gen);
	}
	void clearCmd(const short x, const short y, const short countX, const short countY, const bool firstOrEndUse) {
		if (firstOrEndUse) {
			system("cls");
		}
		else {
			myAssert(0 <= x && 0 <= y && 0 <= countX && 0 <= countY);
			std::string spaceText = std::string(1 + countX, ' ');
			for (short i = 0;i <= countY;i++) {
				myCout(spaceText, x, y + i);
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
		cursorBox.setPosition(cb.getPositionX(), cb.getPositionY());
		cursorBox.setEvent(cb.getEventX(), cb.getEventY());
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