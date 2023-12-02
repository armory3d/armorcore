package kha;

class Framebuffer implements Canvas {
	var window: Int;
	var graphics2: kha.graphics2.Graphics;
	var graphics4: kha.graphics4.Graphics;

	public function new(window: Int, g2: kha.graphics2.Graphics, g4: kha.graphics4.Graphics) {
		this.window = window;
		this.graphics2 = g2;
		this.graphics4 = g4;
	}

	public function init(g2: kha.graphics2.Graphics, g4: kha.graphics4.Graphics): Void {
		this.graphics2 = g2;
		this.graphics4 = g4;
	}

	public var g2(get, never): kha.graphics2.Graphics;
	private function get_g2(): kha.graphics2.Graphics {
		return graphics2;
	}

	public var g4(get, never): kha.graphics4.Graphics;
	private function get_g4(): kha.graphics4.Graphics {
		return graphics4;
	}

	public var width(get, null): Int;
	function get_width(): Int {
		return System.windowWidth(window);
	}

	public var height(get, null): Int;
	function get_height(): Int {
		return System.windowHeight(window);
	}
}
