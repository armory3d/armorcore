package kha.input;

@:allow(kha.System)
class Mouse {

	var windowDownListeners: Array<Int->Int->Int->Void> = [];
	var windowUpListeners: Array<Int->Int->Int->Void> = [];
	var windowMoveListeners: Array<Int->Int->Int->Int->Void> = [];
	var windowWheelListeners: Array<Int->Void> = [];

	private function new() {}

	public static function get(): Mouse {
		return System.getMouse();
	}

	public function notify(downListener: Int->Int->Int->Void, upListener: Int->Int->Int->Void, moveListener: Int->Int->Int->Int->Void, wheelListener: Int->Void, leaveListener:Void->Void = null): Void {
		windowDownListeners.push(downListener);
		windowUpListeners.push(upListener);
		windowMoveListeners.push(moveListener);
		windowWheelListeners.push(wheelListener);
	}

	public function lock(): Void {
		System.lockMouse();
	}

	public function unlock(): Void {
		System.unlockMouse();
	}

	public function canLock(): Bool {
		return System.canLockMouse();
	}

	public function isLocked(): Bool {
		return System.isMouseLocked();
	}

	public function hideSystemCursor(): Void {
		System.hideSystemCursor();
	}

	public function showSystemCursor(): Void {
		System.showSystemCursor();
	}

	private function sendDownEvent(button: Int, x: Int, y: Int): Void {
		if (windowDownListeners != null) {
			for (listener in windowDownListeners) {
				listener(button, x, y);
			}
		}
	}

	private function sendUpEvent(button: Int, x: Int, y: Int): Void {
		if (windowUpListeners != null) {
			for (listener in windowUpListeners) {
				listener(button, x, y);
			}
		}
	}

	private function sendMoveEvent(x: Int, y: Int, movementX: Int, movementY: Int): Void {
		if (windowMoveListeners != null) {
			for (listener in windowMoveListeners) {
				listener(x, y, movementX, movementY);
			}
		}
	}

	private function sendWheelEvent(delta: Int): Void {
		if (windowWheelListeners != null) {
			for (listener in windowWheelListeners) {
				listener(delta);
			}
		}
	}
}
