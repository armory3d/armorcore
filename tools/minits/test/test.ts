
// ../../../make --run

type point_t = {
	x: f32;
	y: f32;
};

function add(point: point_t) {
	point.x += 2.0;
}

function get_five(): i32 {
	let five: ()=>i32 = function (): i32 {
		return 5;
	}
	return five();
}

function main() {
	let p: point_t = { x: 1.5, y: 3.5 };
	add(p);

	let ar: f32_array_t = f32_array_create(0);
	f32_array_push(ar, p.x);

	for (let i: i32 = 0; i < 4; ++i) {
		ar[0] += i;
	}

	while (true) {
		ar[0] += get_five();
		break;
	}

	// Print out result: 14.5
	kinc_log(KINC_LOG_LEVEL_INFO, "%f", ar[0]);
	exit(1);
}
