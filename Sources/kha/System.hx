package kha;

import kha.WindowOptions;

@:structInit
class SystemOptions {
	@:optional public var title: String = "Kha";
	@:optional public var width: Int = -1;
	@:optional public var height: Int = -1;
	@:optional public var window: WindowOptions = null;
	@:optional public var framebuffer: FramebufferOptions = null;

	/**
	 * Used to provide parameters for System.start
	 * @param title The application title is the default window title (unless the window parameter provides a title of its own)
	 * and is used for various other purposes - for example for save data locations
	 * @param width Just a shortcut which overwrites window.width if set
	 * @param height Just a shortcut which overwrites window.height if set
	 * @param window Optionally provide window options
	 * @param framebuffer Optionally provide framebuffer options
	 */
	public function new(title: String = "Kha", ?width: Int = -1, ?height: Int = -1, window: WindowOptions = null, framebuffer: FramebufferOptions = null) {
		this.title = title;
		this.window = window == null ? {} : window;

		if (width > 0) {
			this.window.width = width;
			this.width = width;
		}
		else {
			this.width = this.window.width;
		}

		if (height > 0) {
			this.window.height = height;
			this.height = height;
		}
		else {
			this.height = this.window.height;
		}

		if (this.window.title == null) {
			this.window.title = title;
		}

		this.framebuffer = framebuffer == null ? {} : framebuffer;
	}
}

@:allow(kha.SystemImpl)
class System {
	static var renderListeners: Array<Array<Framebuffer> -> Void> = [];
	static var foregroundListeners: Array<Void -> Void> = [];
	static var resumeListeners: Array<Void -> Void> = [];
	static var pauseListeners: Array<Void -> Void> = [];
	static var backgroundListeners: Array<Void -> Void> = [];
	static var shutdownListeners: Array<Void -> Void> = [];
	static var dropFilesListeners: Array<String -> Void> = [];
	static var cutListener: Void->String = null;
	static var copyListener: Void->String = null;
	static var pasteListener: String->Void = null;

	public static function start(options: SystemOptions, callback: Window -> Void): Void {
		SystemImpl.init(options, callback);
	}

	/**
	 * The provided listener is called when new framebuffers are ready for rendering into.
	 * Each framebuffer corresponds to the kha.Window of the same index, single-window
	 * applications always receive an array of only one framebuffer.
	 * @param listener
	 * The callback to add
	 */
	public static function notifyOnFrames(listener: Array<Framebuffer> -> Void): Void {
		renderListeners.push(listener);
	}

	/**
	 * Removes a previously set frames listener.
	 * @param listener
	 * The callback to remove
	 */
	public static function removeFramesListener(listener: Array<Framebuffer> -> Void): Void {
		renderListeners.remove(listener);
	}

	public static function notifyOnApplicationState(foregroundListener: Void -> Void, resumeListener: Void -> Void,	pauseListener: Void -> Void, backgroundListener: Void-> Void, shutdownListener: Void -> Void): Void {
		if (foregroundListener != null) foregroundListeners.push(foregroundListener);
		if (resumeListener != null) resumeListeners.push(resumeListener);
		if (pauseListener != null) pauseListeners.push(pauseListener);
		if (backgroundListener != null) backgroundListeners.push(backgroundListener);
		if (shutdownListener != null) shutdownListeners.push(shutdownListener);
	}

	public static function notifyOnDropFiles(dropFilesListener: String -> Void): Void {
		dropFilesListeners.push(dropFilesListener);
	}

	public static function removeDropListerer(listener: String -> Void): Void {
		dropFilesListeners.remove(listener);
	}

	public static function notifyOnCutCopyPaste(cutListener: Void->String, copyListener: Void->String, pasteListener: String->Void): Void {
		System.cutListener = cutListener;
		System.copyListener = copyListener;
		System.pasteListener = pasteListener;
	}

	static function render(framebuffers: Array<Framebuffer>): Void {
		for (listener in renderListeners) {
			listener(framebuffers);
		}
	}

	private static function foreground(): Void {
		for (listener in foregroundListeners) {
			listener();
		}
	}

	private static function resume(): Void {
		for (listener in resumeListeners) {
			listener();
		}
	}

	private static function pause(): Void {
		for (listener in pauseListeners) {
			listener();
		}
	}

	private static function background(): Void {
		for (listener in backgroundListeners) {
			listener();
		}
	}

	private static function shutdown(): Void {
		for (listener in shutdownListeners) {
			listener();
		}
	}

	private static function dropFiles(filePath: String): Void {
		for (listener in dropFilesListeners) {
			listener(filePath);
		}
	}

	public static var time(get, null): Float;

	private static function get_time(): Float {
		return SystemImpl.getTime();
	}

	public static function windowWidth(window: Int = 0): Int {
		return Window.get(window).width;
	}

	public static function windowHeight(window: Int = 0): Int {
		return Window.all[window].height;
	}

	public static var systemId(get, null): String;

	static function get_systemId(): String {
		return SystemImpl.getSystemId();
	}

	/**
	 * Pulses the vibration hardware on the device for time in milliseconds, if such hardware exists.
	 */
	public static function vibrate(ms:Int): Void {
		return SystemImpl.vibrate(ms);
	}

	/**
	 * The IS0 639 system current language identifier.
	 */
	public static var language(get, never): String;

	private static function get_language(): String {
		return SystemImpl.getLanguage();
	}

	/**
	 * Schedules the application to stop as soon as possible. This is not possible on all targets.
	 * @return Returns true if the application can be stopped
	 */
	public static function stop(): Bool {
		return SystemImpl.requestShutdown();
	}

	public static function loadUrl(url: String): Void {
		SystemImpl.loadUrl(url);
	}

	public static function safeZone(): Float {
		return SystemImpl.safeZone();
	}
}
