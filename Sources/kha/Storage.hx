package kha;

import haxe.io.Bytes;
import haxe.io.BytesData;
import haxe.Serializer;
import haxe.Unserializer;
using StringTools;

class StorageFile {

	private var name: String;

	public function new(name: String) {
		this.name = name;
	}

	public function read(): Blob {
		var data: BytesData = Krom.readStorage(name);
		return data != null ? Blob.fromBytes(Bytes.ofData(data)) : null;
	}

	public function write(data: Blob): Void {
		if (data != null) {
			Krom.writeStorage(name, data.toBytes().getData());
		}
	}

	public function append(data: Blob): Void {

	}

	public function canAppend(): Bool {
		return false;
	}

	public function maxSize(): Int {
		return -1;
	}

	public function writeString(data: String): Void {
		var bytes = Bytes.ofString(data);
		write(Blob.fromBytes(bytes));
	}

	public function appendString(data: String): Void {
		var bytes = Bytes.ofString(data);
		append(Blob.fromBytes(bytes));
	}

	public function readString(): String {
		var blob = read();
		if (blob == null) return null;
		else return blob.toString();
	}

	public function writeObject(object: Dynamic): Void {
		writeString(Serializer.run(object));
	}

	public function readObject(): Dynamic {
		var s = readString();
		if (s == null) return null;
		try {
			return Unserializer.run(s);
		}
		catch (e: Dynamic) {
			return null;
		}
	}
}

class Storage {
	public static function namedFile(name: String): StorageFile {
		name = name.replace("\\", ".");
		name = name.replace("/", ".");
		return new StorageFile(name);
	}

	public static function defaultFile(): StorageFile {
		return namedFile("default.kha");
	}
}
