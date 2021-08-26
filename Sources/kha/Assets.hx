package kha;

import kha.Blob;
import kha.Font;
import kha.Sound;
import haxe.io.Bytes;
import haxe.io.BytesData;

class Assets {

	public static function loadImageFromPath(path: String, readable: Bool, done: Image -> Void): Void {
		var image = Krom.loadImage(path, readable);
		if (image != null) done(Image._fromTexture(image));
	}

	public static function loadBlobFromPath(path: String, done: Blob -> Void): Void {
		done(new Blob(Bytes.ofData(Krom.loadBlob(path))));
	}

	public static function loadSoundFromPath(path: String, done: Sound -> Void): Void {
		done(new Sound(Bytes.ofData(Krom.loadSound(path))));
	}

	public static function loadFontFromPath(path: String, done: Font -> Void): Void {
		loadBlobFromPath(path, function (blob: Blob) {
			done(new Font(blob));
		});
	}

	public static function loadVideoFromPath(path: String, done: Video -> Void): Void {

	}
}
