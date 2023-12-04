package kha;

interface Canvas {
	var width(get, null): Int;
	var height(get, null): Int;
	var g2(get, null): kha.Graphics2;
	var g4(get, null): kha.Graphics4;
}
