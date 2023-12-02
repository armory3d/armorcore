package kha.input;

@:allow(kha.System)
class Pen {

	var windowDownListeners: Array<Int->Int->Float->Void> = [];
	var windowUpListeners: Array<Int->Int->Float->Void> = [];
	var windowMoveListeners: Array<Int->Int->Float->Void> = [];

	private function new() {}

	public static function get(): Pen {
		return System.getPen();
	}

	public function notify(downListener: Int->Int->Float->Void, upListener: Int->Int->Float->Void, moveListener: Int->Int->Float->Void): Void {
		windowDownListeners.push(downListener);
		windowUpListeners.push(upListener);
		windowMoveListeners.push(moveListener);
	}

	private function sendDownEvent(x: Int, y: Int, pressure: Float): Void {
		if (windowDownListeners != null) {
			for (listener in windowDownListeners) {
				listener(x, y, pressure);
			}
		}
	}

	private function sendUpEvent(x: Int, y: Int, pressure: Float): Void {
		if (windowUpListeners != null) {
			for (listener in windowUpListeners) {
				listener(x, y, pressure);
			}
		}
	}

	private function sendMoveEvent(x: Int, y: Int, pressure: Float): Void {
		if (windowMoveListeners != null) {
			for (listener in windowMoveListeners) {
				listener(x, y, pressure);
			}
		}
	}
}
