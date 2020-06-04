package kha;

class Assets {

	public static function loadImageFromPath(path: String, readable: Bool, done: Image -> Void): Void {
		var description = { files: [ path ], readable: readable };
		LoaderImpl.loadImageFromDescription(description, done);
	}

	public static function loadBlobFromPath(path: String, done: Blob -> Void): Void {
		var description = { files: [ path ] };
		LoaderImpl.loadBlobFromDescription(description, done);
	}

	public static function loadSoundFromPath(path: String, done: Sound -> Void): Void {
		var description = { files: [ path ] };
		LoaderImpl.loadSoundFromDescription(description, done);
	}

	public static function loadFontFromPath(path: String, done: Font -> Void): Void {
		var description = { files: [ path ] };
		LoaderImpl.loadFontFromDescription(description, done);
	}

	public static function loadVideoFromPath(path: String, done: Video -> Void): Void {
		var description = { files: [ path ] };
		LoaderImpl.loadVideoFromDescription(description, done);
	}
}
