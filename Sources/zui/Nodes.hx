package zui;

class Nodes {

	public static var excludeRemove: Array<String> = [];
	public static var onLinkDrag: TNodeLink->Bool->Void = null;
	public static var onSocketReleased: TNodeSocket->Void = null;
	public static var onCanvasReleased: Void->Void = null;
	public static var onNodeRemove: TNode->Void = null;
	public static var onCanvasControl: Void->CanvasControl = null;
	public static var enumTexts: String->Array<String> = null;
	public static var socketReleased = false;
	public static var clipboard = "";
	public var nodesDrag = false;
	public var nodesSelected: Array<TNode> = [];
	public var panX = 0.0;
	public var panY = 0.0;
	public var zoom = 1.0;
	public var _inputStarted = false;
	public var colorPickerCallback: kha.Color->Void = null;
	public var linkDrag: TNodeLink = null;
	public var handle = new Zui.Handle();

	var nodes_: Dynamic;

	public function new() {
		nodes_ = Krom.zui_nodes_init();
	}

	public static dynamic function tr(id: String, vars: Map<String, String> = null) {
		return id;
	}

	public function p(f: Float): Float {
		return 1;
	}

	public function getNode(nodes: Array<TNode>, id: Int): TNode {
		return null;
	}

	public function getNodeId(nodes: Array<TNode>): Int {
		return 0;
	}

	public function getLinkId(links: Array<TNodeLink>): Int {
		return 0;
	}

	public function getSocketId(nodes: Array<TNode>): Int {
		return 0;
	}

	public function nodeCanvas(ui: Zui, canvas: TNodeCanvas) {
		// Krom.zui_node_canvas();
	}

	public function rgbaPopup(ui: Zui, nhandle: zui.Zui.Handle, val: kha.arrays.Float32Array, x: Int, y: Int) {

	}

	public function removeNode(n: TNode, canvas: TNodeCanvas) {

	}

	public function NODE_H(canvas: TNodeCanvas, node: TNode): Int {
		return 1;
	}
	public function NODE_W(node: TNode): Int {
		return 1;
	}
	public function NODE_X(node: TNode): Float {
		return 1;
	}
	public function NODE_Y(node: TNode): Float {
		return 1;
	}
	public function BUTTONS_H(node: TNode): Int {
		return 1;
	}
	public function OUTPUTS_H(sockets: Array<TNodeSocket>, length: Null<Int> = null): Int {
		return 1;
	}
	public function INPUT_Y(canvas: TNodeCanvas, sockets: Array<TNodeSocket>, pos: Int): Int {
		return 1;
	}
	public function OUTPUT_Y(sockets: Array<TNodeSocket>, pos: Int): Int {
		return 1;
	}
	public function SCALE(): Float {
		return 1;
	}
	public function PAN_X(): Float {
		return 1;
	}
	public function PAN_Y(): Float {
		return 1;
	}
	public function LINE_H(): Int {
		return 1;
	}
}

typedef CanvasControl = {
	var panX: Float;
	var panY: Float;
	var zoom: Float;
}

typedef TNodeCanvas = {
	var name: String;
	var nodes: Array<TNode>;
	var links: Array<TNodeLink>;
}

typedef TNode = {
	var id: Int;
	var name: String;
	var type: String;
	var x: Float;
	var y: Float;
	var inputs: Array<TNodeSocket>;
	var outputs: Array<TNodeSocket>;
	var buttons: Array<TNodeButton>;
	var color: Int;
	@:optional var width: Null<Float>;
	@:optional var tooltip: String;
}

typedef TNodeSocket = {
	var id: Int;
	var node_id: Int;
	var name: String;
	var type: String;
	var color: Int;
	var default_value: Dynamic;
	@:optional var min: Null<Float>;
	@:optional var max: Null<Float>;
	@:optional var precision: Null<Float>;
	@:optional var display: Null<Int>;
	@:optional var tooltip: String;
}

typedef TNodeLink = {
	var id: Int;
	var from_id: Int;
	var from_socket: Int;
	var to_id: Int;
	var to_socket: Int;
}

typedef TNodeButton = {
	var name: String;
	var type: String;
	@:optional var output: Null<Int>;
	@:optional var default_value: Dynamic;
	@:optional var data: Dynamic;
	@:optional var min: Null<Float>;
	@:optional var max: Null<Float>;
	@:optional var precision: Null<Float>;
	@:optional var height: Null<Float>;
	@:optional var tooltip: String;
}
