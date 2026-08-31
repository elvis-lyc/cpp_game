#include "myTool.hpp"
#include "snake.hpp"
#include "gameCenter.hpp"

namespace gameCenter {
	namespace {
		void initGame() {
			myTool::clearCmd({ 0,0 }, { 0,0 }, true);
			myTool::setCmdTitle(L"Game Application 3.0");
			myTool::setCursorState(false);
			myTool::startCursor();
			myTool::startKeyInput();
		}
		void layout() {
			myTool::myCout("歡迎使用Game Application 3.0，請以滑鼠點擊所需功能", { 0,0 }, 7);
			myTool::myCout("貪吃蛇遊戲", { 0,2 });
			myTool::myCout("結束程序", { 0,4 });
		}
		void clearLayout() {
			myTool::clearCmd({ 0,0 }, { 49,4 });
		}
		void stopGame() {
			myTool::endCursor();
			myTool::endKeyInput();
			myTool::setCursorState(true);
			myTool::clearCmd({ 0,0 }, { 0,0 }, true);
		}
	}
	void start() {
		initGame();
		layout();
		while (true) {
			myTool::resetCursor();
			myTool::mySleep(200);
			myTool::getCursor();
			myTool::colorChangeLayout("貪吃蛇遊戲", { 0,2 }, { 9,0 }, 2);
			myTool::colorChangeLayout("結束程序", { 0,4 }, { 7,0 }, 2);
			if (myTool::cursorBox.getState() && myTool::cursorBox.getOneClick() &&
				myTool::cursorBox.getLeftPressed()) {
				if (myTool::cursorTouchArea({ 0,2 }, { 9,0 })) {
					clearLayout();
					snake::start();
					layout();
				}
				else if (myTool::cursorTouchArea({ 0,4 }, { 7,0 })) {
					break;
				}
			}
		}
		myTool::setCursorColor();
		stopGame();
	}
}