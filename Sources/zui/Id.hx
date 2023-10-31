package zui;

import zui.Zui;

class Id {

	public static var children: Map<String, Handle> = [];

	public static function handle(s: String, ops: HandleOptions = null): Handle {
		var h = children.get(s);
		if (h == null) {
			h = new Handle(ops);
			children.set(s, h);
		}
		return h;
	}
}
