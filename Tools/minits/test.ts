
type point_t = {
	x: f32;
	y: f32;
};

function add(point: point_t) {
	point.x += 2.0;
}

function get_five(): i32 {
	return 5;
}

function start() {
	let p: point_t = {};
	p.x = 1.5;
	p.y = 3.0;

	add(p);

	let ar: f32_array_t = [];
	f32_array_push(ar, p.x);
	f32_array_push(ar, p.y);

	for (let i: i32 = 0; i < 4; ++i) {
		if (i == 1) {
			continue;
		}
		else {
			ar[0] += i;
		}
	}

	while (true) {
		ar[0] += get_five();
		break;
	}

	// Print out result: 13.5
	// krom_log(ar[0]);
}
