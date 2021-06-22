// TODO: Move to Kinc

public static byte[] androidHttpRequest(String address) {
	// https://developer.android.com/reference/java/net/HttpURLConnection.html
	URL url = new URL(address);
	HttpURLConnection urlConnection = (HttpURLConnection)url.openConnection();
	InputStream in = new BufferedInputStream(urlConnection.getInputStream());
	byte[] array = in.readAllBytes();
	urlConnection.disconnect();
	return array;
}
