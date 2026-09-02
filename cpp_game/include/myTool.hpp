#pragma once

#include <source_location>
#include <string>
#include <limits>
namespace myTool {
	/*
	自訂static_cast並加入條件判斷
	*/
	template<typename T, typename F>
	T safe_cast(F value);

	struct Point {
		short x = 0, y = 0;
		Point() = default;
		Point(const short inX, const short inY) :x(inX), y(inY) {}

		bool operator==(const Point other) {
			return this->x == other.x && this->y == other.y;
		}
		Point operator+(const Point other) {
			return { safe_cast<short>(this->x + other.x),safe_cast<short>(this->y + other.y) };
		}
	};

	class Tree {
	private:
		struct Node {
			short data = 0;
			Node* left = nullptr, * right = nullptr;
			Node(const short inData):data(inData){}
		};
		Node* _root = nullptr;
		void deleteTree_temp(Tree::Node* node);
		void findCanUseIndex_temp(Tree::Node* node, short& index);
	public:
		void insert(const short data);
		void remove(const short data);
		void deleteTree();
		void findCanUseIndex(short& index);
	};

	class CursorBox {
	private:
		Point _position{ 0,0 }, _event{ 0,0 };
		/*
		Bitset
		0~4: _state, _oneClick, _doubleClick, _leftPressed, _rightPressed
		*/
		char _bit8 = 0;
	public:
		bool getState() const;
		bool getOneClick() const;
		bool getDoubleClick() const;
		bool getLeftPressed() const;
		bool getRightPressed() const;
		void set(const bool state, const bool oneClk, const bool douClk,
			const bool lPre, const bool rPre);
		short getPositionX() const;
		short getPositionY() const;
		short getEventX() const;
		short getEventY() const;
		void setPosition(const Point point);
		void setEvent(const Point point);
	};
	inline CursorBox cursorBox;

	/*
	自訂斷言
	傳入條件，條件為 False 時中止程序
	*/
	void myAssert(const bool condition, const std::string& errorText,
		const std::source_location& loc = std::source_location::current());

	/*
	設定 Cmd 光標位置
	傳入值為空:(x,y)=>(0,0)
	*/
	void setCursorXY(const Point point);

	/*
	設定 Cmd 光標輸出顏色
	傳入值為空:(c)=>7(白色)
	*/
	void setCursorColor(const short colorIndex = 7);

	/*
	設定 Cmd 光標位置與輸出顏色
	傳入值為空:(x,y)=>(0,0) (c)=>7(白色)
	*/
	void setCursorXYC(const Point point, const short colorIndex = 7);

	/*
	輸出文字
	*/
	void myCout(const std::string& text);

	/*
	改變位置後輸出文字
	傳入值為空:(x,y)=>(0,0)
	*/
	void myCout(const std::string& text, const Point point);

	/*
	改變位置與輸出顏色後輸出文字
	傳入值為空:(x,y)=>(0,0) (c)=>7(白色)
	*/
	void myCout(const std::string& text, const Point point, const short colorIndex);

	/*
	當光標在矩形範圍內時回傳true
	x、y: 為起始格
	countX、countY:為在起始格的基礎上向該方向再延長數格
	*/
	bool cursorAtArea(const Point point, const Point countPoint);

	/*
	當光標在矩形範圍內時回傳true
	x、y: 為起始格
	countX、countY:為在起始格的基礎上向該方向再延長數格
	*/
	bool cursorAtArea(const Point point, const Point countPoint);

	/*
	當光標在矩形範圍內點擊時回傳true
	x、y: 為起始格
	countX、countY:為在起始格的基礎上向該方向再延長數格
	*/
	bool cursorTouchArea(const Point point, const Point countPoint);

	/*
	在(x,y)輸出name
	當鼠標在{
	x、y: 為起始格
	countX、countY:為在起始格的基礎上向該方向再延長數格}
	的矩形範圍內時
	將name顏色設為changeColor
	反之設為白色(7)
	*/
	void colorChangeLayout(const std::string& name,
		const Point point, const Point countPoint,
		const short changeColor);

	/*
	停滯 ms 毫秒
	*/
	void mySleep(const int ms);

	/*
	從 firstNum ~ endNum 中隨機返回一數字
	*/
	int myRand(const int firstNum, const int endNum);

	/*
	以矩形範圍的空格字元清空畫面
	x、y: 為起始格
	countX、countY:為在起始格的基礎上向該方向再延長數格
	firstOrEndNumUse: 使用system("cls"){會造成監聽功能失效，應只在程式的開頭與結尾使用}
	*/
	void clearCmd(const Point point, const Point countPoint, const bool firstOrEndUse = false);

	/*
	設定 Cmd 頁面名稱
	*/
	void setCmdTitle(const std::wstring& name);

	/*
	設定光標是否能看見
	能看見:true 隱藏:false
	*/
	void setCursorState(const bool canSee);

	/*
	開啟負責監聽滑鼠狀態的執行緒
	*/
	void startCursor();

	/*
	結束負責監聽滑鼠狀態的執行緒
	*/
	void endCursor();

	/*
	將監聽到的滑鼠資訊導入到 myTool::CursorBox cursorBox 的類中
	*/
	void getCursor();

	/*
	重滑鼠狀態紀錄
	*/
	void resetCursor();

	/*
	開啟負責監聽鍵盤狀態的執行緒
	*/
	void startKeyInput();

	/*
	結束負責監聽鍵盤狀態的執行緒
	*/
	void endKeyInput();

	/*
	讀取相應按鍵是否被按下
	已收錄:
	ESC: 0x1B, Space: 0x20, LShift: 0xA0,
	Up: 0x26, Right: 0c27, Down: 0x28, Left: 0x25,
	W: 'W', D: 'D', S: 'S', A: 'A'
	*/
	bool getKeyInput(const int vKey);

	/*
	重製鍵盤狀態紀錄
	*/
	void resetKeyInput();

	/*
	safe_cast實作
	*/
	template<typename T, typename F>
	T safe_cast(F value) {
		myTool::myAssert(std::numeric_limits<T>::min() <= value && value <= std::numeric_limits<T>::max(), "值溢出，無法進行型別轉換");
		return static_cast<T>(value);
	}
}