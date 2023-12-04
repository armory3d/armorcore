package kha;

class Framebuffer implements Canvas {
	var graphics2: kha.Graphics2;
	var graphics4: kha.Graphics4;

	public function new(g2: kha.Graphics2, g4: kha.Graphics4) {
		this.graphics2 = g2;
		this.graphics4 = g4;
	}

	public function init(g2: kha.Graphics2, g4: kha.Graphics4): Void {
		this.graphics2 = g2;
		this.graphics4 = g4;
	}

	public var g2(get, never): kha.Graphics2;
	private function get_g2(): kha.Graphics2 {
		return graphics2;
	}

	public var g4(get, never): kha.Graphics4;
	private function get_g4(): kha.Graphics4 {
		return graphics4;
	}

	public var width(get, null): Int;
	function get_width(): Int {
		return System.windowWidth();
	}

	public var height(get, null): Int;
	function get_height(): Int {
		return System.windowHeight();
	}
}
