
// ts to c:
// ../../Kinc/make --kfile minits.js
// build c:
// ../../Kinc/make --run

let flags = globalThis.flags;
if (flags == null) {
	flags = {};
	flags.minits_source = null;
	flags.minits_input = "./test.ts";
	flags.minits_output = "./test.c";
}

// ██████╗      █████╗     ██████╗     ███████╗    ███████╗
// ██╔══██╗    ██╔══██╗    ██╔══██╗    ██╔════╝    ██╔════╝
// ██████╔╝    ███████║    ██████╔╝    ███████╗    █████╗
// ██╔═══╝     ██╔══██║    ██╔══██╗    ╚════██║    ██╔══╝
// ██║         ██║  ██║    ██║  ██║    ███████║    ███████╗
// ╚═╝         ╚═╝  ╚═╝    ╚═╝  ╚═╝    ╚══════╝    ╚══════╝

let specials = [":", ";", ",", "(", ")", "[", "]", "{", "}", "<", ">", "!"];
let is_comment = false;
let is_string = false;
let pos = 0;
let tokens = [];

function is_alpha_numeric(code) {
	return (code > 47 && code < 58) || // 0-9
		   (code == 45) 			|| // -
		   (code == 43) 			|| // +
		   (code == 95) 			|| // _
		   (code == 46) 			|| // .
		   (code > 64 && code < 91) || // A-Z
		   (code > 96 && code < 123);  // a-z
}

function is_alpha(code) {
	return (code > 64 && code < 91)  || // A-Z
		   (code > 96 && code < 123) || // a-z
		   (code == 95);				// _
}

function is_numeric(code) {
	return (code > 47 && code < 58); // 0-9
}

function is_number(s) {
	if (is_numeric(s.charCodeAt(0))) { // 0-9
		return true;
	}
	if (s.charAt(0) == "-" && is_numeric(s.charCodeAt(1))) { // -
		return true;
	}
	return false;
}

function is_white_space(code) {
	return code == 32 || // " "
		   code == 9  || // "\t"
		   code == 10;   // "\n"
}

function read_token() {
	// Skip white space
	while (is_white_space(flags.minits_source.charCodeAt(pos))) {
		pos++;
	}

	let token = "";

	while (pos < flags.minits_source.length) {
		let c = flags.minits_source.charAt(pos);

		// Comment start
		if (token == "//") {
			is_comment = true;
		}

		if (is_comment) {
			token += c;
			pos++;

			// Comment end
			if (c == "\n") {
				is_comment = false;
				token = null; // Remove comments
				break;
			}

			// Prevent parsing of comments
			continue;
		}

		// String start / end
		if (c == "\"") {
			// Escaped \"
			let is_escaped = token == "\"\\";

			if (!is_escaped) {
				is_string = !is_string;
			}

			// Token end - string end
			if (!is_string) {
				token += c;
				pos++;
				break;
			}
		}

		if (is_string) {
			token += c;
			pos++;

			// Prevent parsing of strings
			continue;
		}

		// Token end
		if (is_white_space(flags.minits_source.charCodeAt(pos))) {
			break;
		}

		// Parsing an alphanumeric token
		if (token.length > 0 &&
			is_alpha_numeric(token.charCodeAt(0))) {

			// Token end
			if (specials.indexOf(c) > -1) {
				break;
			}
		}

		// Add char to token and advance pos
		token += c;
		pos++;

		// Token end, got special
		if (specials.indexOf(c) > -1) {
			break;
		}
	}

	return token;
}

function parse() {
	tokens = [];
	pos = 0;

	while (true) {
		let token = read_token();
		if (token == null) { // Throw away this token
			continue;
		}
		if (token.startsWith("//")) { // Throw away comment tokens
			continue;
		}
		if (token == "=" && tokens[tokens.length - 1] == "!") { // Merge "!" and "=" into "!=" token
			tokens[tokens.length - 1] = "!=";
			continue;
		}
		if (token == "") { // No more tokens
			break;
		}
		tokens.push(token);
	}

	return tokens;
}

// ██╗         ██╗    ██████╗
// ██║         ██║    ██╔══██╗
// ██║         ██║    ██████╔╝
// ██║         ██║    ██╔══██╗
// ███████╗    ██║    ██████╔╝
// ╚══════╝    ╚═╝    ╚═════╝

let stream;
let string = "";
let tabs = 0;
let new_line = false;
let basic_types = ["i8", "u8", "i16", "u16", "i32", "u32", "f32", "bool"];
let enums = [];
let value_types = new Map();
let struct_types = new Map();
let fn_declarations = new Map();
let fn_default_params = new Map();
let fn_call_stack = [];
let param_pos_stack = [];
let is_for_loop = false;
let global_inits = [];
let global_ptrs = [];

function handle_tabs(token) {
	// Entering block, add tab
	if (token == "{") {
		tabs++;
	}
	else if (token == "}") {
		tabs--;
	}

	if (is_for_loop) {
		return;
	}

	// New line, add tabs
	if (new_line) {
		for (let i = 0; i < tabs; ++i) {
			write("\t");
		}
	}
}

function handle_new_line(token) {
	if (is_for_loop) {
		return;
	}

	// Insert new line
	new_line = token == ";" || token == "{" || token == "}";
	if (new_line) {
		write("\n");
	}
}

function handle_spaces(token, keywords) {
	// Add space to separate keywords
	for (let kw of keywords) {
		if (token == kw) {
			write(" ");
			break;
		}
	}
}

function get_token(off = 0) {
	if (pos + off < tokens.length) {
		return tokens[pos + off];
	}
	return "";
}

function get_token_after_piece() {
	let _pos = pos;
	read_piece();
	let t = get_token(1);
	pos = _pos;
	return t;
}

function skip_until(s) {
	while (true) {
		let token = get_token();
		if (token == s) {
			break;
		}
		pos++;
	}
}

function skip_block() { // { ... }
	pos++; // {
	let nested = 1;
	while (true) {
		let token = get_token();
		if (token == "}") {
			nested--;
			if (nested == 0) {
				break;
			}
		}
		else if (token == "{") {
			nested++;
		}
		pos++;
	}
}

function function_return_type() {
	let _pos = pos;
	skip_until("{");
	if (get_token(-1) == "]") { // ): i32[]
		pos -= 2;
	}
	pos -= 2; // ): i32 {
	let result = get_token() == ":" ? read_type() : "void";
	pos = _pos;
	return result;
}

function join_type_name(type, name) {
	value_types.set(name, type);
	if (type.indexOf("(*NAME)(") > 0) { // Function pointer
		return type.replace("NAME", name);
	}
	return type + " " + name;
}

function read_type() { // Cursor at ":"
	pos++;
	let type = get_token();

	if (type == "(" && get_token(1) == "(") { // (()=>void)[]
		pos++;
	}

	if (get_token(1) == ")" && get_token(2) == "[") { // (()=>void)[]
		pos++;
	}

	if (type == "(") { // ()=>void
		let params = "(";

		if (get_token(1) != ")") { // Has params
			while (true) {
				skip_until(":");
				params += read_type();

				pos++;
				if (get_token() == ")") { // End of params
					break;
				}

				params += ",";
			}
		}
		else {
			params += "void";
		}

		params += ")";
		skip_until("=>");
		let ret = read_type();
		type = ret + "(*NAME)" + params;
	}

	if (type == "color_t") {
		type = "i32";
	}

	if (type == "string") {
		type = "string_t";
	}

	if (get_token(1) == "[") { // Array
		if (is_struct(type)) {
			type = type + "_array_t";
		}
		else if (type == "bool") {
			type = "u8_array_t";
		}
		else {
			type = type + "_array_t";
		}
		pos += 2; // Skip "[]"
	}

	if (is_struct(type)) {
		type += " *";
	}

	if (type == "map_t *" && get_token(1) == "<") { // Map
		let t = get_token(4);
		if (t == "f32") {
			type = "f32_" + type;
		}
		else if (basic_types.indexOf(t) > -1) {
			type = "i32_" + type;
		}
		else {
			type = "any_" + type;
		}
		pos += 5; // Skip <a, b>
	}

	return type;
}

function enum_access(s) {
	// Turn enum_t.VALUE into enum_t_VALUE
	for (let e of enums) {
		if (s.indexOf(e + ".") > -1 && is_alpha(s.charCodeAt(0))) {
			s = s.replaceAll(e + ".", e + "_");
		}
	}
	return s;
}

function struct_access(s) {
	// Turn a.b into a->b
	if (s.indexOf(".") > -1 && !is_number(s) && s.charAt(0) != "\"" && !s.startsWith("GC_ALLOC_INIT")) {
		s = s.replaceAll(".", "->");
	}
	return s;
}

function struct_alloc(token, type = null) {
	// "= { a: b, ... }" -> GC_ALLOC_INIT
	if ((get_token(-1) == "=" && token == "{") || type != null) {
		if (type == null) {
			// let a: b = {, a = {
			let i = get_token(-3) == ":" ? -4 : -2;
			let t = struct_access(get_token(i));
			type = value_type(t);
			if (type.endsWith(" *")) { // my_struct * -> my_struct
				type = type.substring(0, type.length - 2);
			}
		}

		token = "GC_ALLOC_INIT(" + type + ", {";
		pos++; // {
		while (get_token() != "}") {
			let t = get_token_c();
			if (get_token() == "[") {
				let member = get_token(-2);
				let types = struct_types.get(type + " *");
				let content_type = types.get(member);
				content_type = content_type.substring(7, content_type.length - 8); // struct my_struct_array_t * -> my_struct
				t = array_create("[", "", content_type);
			}
			token += t;
			pos++;
		}
		token += get_token(); // "}"
		token += ")";
		tabs--;
	}
	return token;
}

function array_type(name) {
	let type = value_type(name);
	if (type == null) {
		return "any";
	}
	if (type.startsWith("struct ")) { // struct u8_array * -> u8_array_t *
		type = type.substring(7, type.length - 2) + "_t *";
	}
	type = strip(type, 10); // Strip _array_t *
	if (basic_types.indexOf(type) > -1) { // u8
		return type;
	}
	else {
		return "any";
	}
}

function array_contents(type) {
	// [1, 2, ..] or [{a:b}, {a:b}]
	let contents = [];
	while (true) {
		pos++;
		let token = get_token();
		if (token == "]") {
			break;
		}
		if (token == ",") {
			continue;
		}
		if (token == "{") {
			token = struct_alloc(token, type);
			tabs++; // Undo tabs-- in struct_alloc
		}
		token = struct_access(token);
		contents.push(token);
	}
	return contents;
}

function member_name(name) {
	let i = name.lastIndexOf(".");
	if (i > -1) {
		name = name.substring(i + 1, name.length);
	}
	return name;
}

function array_create(token, name, content_type = null) {
	if ((get_token(-1) == "=" && token == "[") || content_type != null) {
		if (name == "]") {
			name = get_token(-6); // ar: i32[] = [
		}
		let member = member_name(name);
		let type = array_type(member);

		// = [], = [1, 2, ..] -> any/i32/.._array_create_from_raw
		if (content_type == null) {
			content_type = value_type(name);
			if (content_type != null) {
				content_type = content_type.substring(0, content_type.length - 10);
			}
		}
		let contents = array_contents(content_type);
		token = type + "_array_create_from_raw((" + type + "[]){";
		for (let e of contents) {
			token += e + ",";
		}
		token += "}," + contents.length + ")";
	}
	return token;
}

function array_access(token) {
	// array[0] -> array->buffer[0]
	if (get_token(-1) != "=" && token == "[") {
		token = "->buffer["
	}
	return token;
}

function is_struct(type) {
	// Ends with _t and is not an enum
	return type.endsWith("_t") && enums.indexOf(type) == -1;
}

function strip(name, len) {
	return name.substring(0, name.length - len);
}

function strip_optional(name) {
	if (name.endsWith("?")) {
		name = strip(name, 1); // :val? -> :val
	}
	return name;
}

function param_pos() {
	return param_pos_stack[param_pos_stack.length - 1];
}

function param_pos_add() {
	param_pos_stack[param_pos_stack.length - 1]++;
}

function fn_call() {
	return fn_call_stack[fn_call_stack.length - 1];
}

function get_filled_fn_params(token) {
	let res = "";

	if (token == "(") {
		// Function is being called to init this variable
		fn_call_stack.push(get_token(-1));
		param_pos_stack.push(0);
	}

	if (token == ",") {
		// Param has beed passed manually
		param_pos_add();
	}

	if (token == ")") {
		// Fill in default parameters if needed
		if (get_token(-1) != "(") {
			// Param has beed passed manually
			param_pos_add();
		}

		// If default param exists, fill it
		while (fn_default_params.has(fn_call() + param_pos())) {
			if (param_pos() > 0) {
				res += ",";
			}
			res += fn_default_params.get(fn_call() + param_pos());
			param_pos_add();
		}

		fn_call_stack.pop();
		param_pos_stack.pop();
	}

	return res;
}

function get_token_c() {
	let t = get_token();
	t = enum_access(t);
	t = struct_access(t);
	t = array_access(t);
	return t;
}

function read_piece(nested = 0) {
	let piece = get_token_c();
	piece = enum_access(piece);
	piece = struct_access(piece);
	piece = array_access(piece);
	piece = string_length(piece);

	if (get_token(1) == "(" || get_token(1) == "[") {
		nested++;
		pos++;
		return piece + read_piece(nested);
	}

	if (get_token(1) == ")" || get_token(1) == "]") {
		nested--;
		if (nested < 0) {
			return piece;
		}
		pos++;
		return piece + read_piece(nested);
	}

	if (nested > 0) {
		pos++;
		return piece + read_piece(nested);
	}

	if (get_token(1).startsWith(".")) {
		pos++;
		return piece + read_piece();
	}

	return piece;
}

function string_length(token) {
	// "str->length" -> "string_length(str)"
	let base = strip(token, 8); // ->length
	base = value_type(base);
	if (base == "struct string *") {
		base = "string_t *";
	}

	if (token.endsWith("->length") && base == "string_t *") {
		token = "string_length(" + strip(token, 8) + ")";
	}
	return token;
}

function value_type(value) {
	if (value.indexOf("->") > -1) {
		let base = value.substring(0, value.indexOf("->"));
		let type = value_types.get(base);
		value = value.substring(value.indexOf("->") + 2, value.length);
		let struct_value_types = struct_types.get(type);
		if (struct_value_types != null) {
			return struct_value_types.get(value);
		}
	}
	return value_types.get(value);
}

function set_fn_param_types() {
	pos++; // (
	while (true) {
		pos++;
		let t = get_token(); // Param name, ) or ,

		if (t == ")") { // Params end
			break;
		}

		if (t == ",") { // Next param
			continue;
		}

		pos++; // :
		let type = read_type();
		value_types.set(t, type);

		if (get_token(1) == "=") { // Skip default value
			pos += 2;
		}
	}
}

function string_ops(token) {
	// "str" + str + 1 + ...
	let first = true;
	while (get_token_after_piece() == "+") {
		if (first) {
			first = false;
			token = read_piece();

			if (value_type(token) == "i32") {
				token = "i32_to_string(" + token + ")";
			}
		}
		pos++;

		token = "string_join(" + token + ",";
		pos++;

		let token_b = read_piece();
		if (value_type(token_b) == "i32") {
			token_b = "i32_to_string(" + token_b + ")";
		}

		token += token_b;
		token += ")";
	}

	// "str" += str
	if (get_token_after_piece(1) == "+=") {
		token = read_piece();
		pos++;

		token = token + "=string_join(" + token + ",";
		pos++;
		token += read_piece();
		token += ")";
	}

	// "str" == str
	if (get_token_after_piece(1) == "==") {
		token = read_piece();
		pos++;

		token = "string_equals(" + token + ",";
		pos++;
		token += read_piece();
		token += ")";
	}

	// "str" != str
	if (get_token_after_piece(1) == "!=") {
		token = read_piece();
		pos++;

		token = "!string_equals(" + token + ",";
		pos++;
		token += read_piece();
		token += ")";
	}

	return token;
}

function map_ops(token) {
	if (token == "map_create") {
		let t = value_type(get_token(-2));
		if (t == "i32_map_t *") {
			token = "i32_map_create";
		}
		else {
			token = "any_map_create";
		}
	}

	if (token == "map_set") {
		let t = value_type(get_token(2));
		if (t == "i32_map_t *") {
			token = "i32_map_set";
		}
		else {
			token = "any_map_set";
		}
	}

	if (token == "map_get") {
		let t = value_type(get_token(2));
		if (t == "i32_map_t *") {
			token = "i32_map_get";
		}
		else {
			token = "any_map_get";
		}
	}

	return token;
}

// ██╗    ██╗    ██████╗     ██╗    ████████╗    ███████╗         ██████╗
// ██║    ██║    ██╔══██╗    ██║    ╚══██╔══╝    ██╔════╝        ██╔════╝
// ██║ █╗ ██║    ██████╔╝    ██║       ██║       █████╗          ██║
// ██║███╗██║    ██╔══██╗    ██║       ██║       ██╔══╝          ██║
// ╚███╔███╔╝    ██║  ██║    ██║       ██║       ███████╗        ╚██████╗
//  ╚══╝╚══╝     ╚═╝  ╚═╝    ╚═╝       ╚═╝       ╚══════╝         ╚═════╝

function stream_write(token) {
	stream.write(token);
}

function string_write(token) {
	string += token;
}

let write = stream_write;

function write_enums() {
	enums = [];
	for (pos = 0; pos < tokens.length; ++pos) {
		let token = get_token();

		// Turn "enum name {}" into "typedef enum {} name;"
		if (token == "enum") {
			pos++;
			let enum_name = get_token();
			enums.push(enum_name);

			write("typedef enum{\n");
			pos++; // {

			while (true) {
				// Enum contents
				pos++;
				token = get_token(); // Item name

				if (token == "}") { // Enum end
					write("}" + enum_name + ";\n");
					break;
				}

				write("\t" + enum_name + "_" + token);

				pos++; // = or ,
				token = get_token();

				if (token == "=") { // Enum value
					pos++; // n
					token = get_token();
					write("=" + token);
					pos++; // ,
				}

				write(",\n");
			}

			write("\n");
			continue;
		}
	}
}

function write_types() {
	for (pos = 0; pos < tokens.length; ++pos) {
		let token = get_token();

		// Turn "type x = {};" into "typedef struct {} x;"
		// Turn "type x = y;" into "typedef x y;"
		if (token == "type") {
			pos++;
			let struct_name = get_token();
			let stuct_name_short = strip(struct_name, 2); // _t

			pos++;
			token = get_token(); // =
			if (token != "=") {
				continue;
			}

			pos++;
			token = get_token();

			// "type x = y;"
			if (token != "{") {
				pos--;
				let type = read_type();

				write("typedef " + type + " " + struct_name + ";\n\n");
				skip_until(";");
				continue;
			}

			// "type x = {};"
			// Use PACK() for armpack support (use only when " _: " is present?)
			write("typedef PACK(struct " + stuct_name_short + "{\n");

			let struct_value_types = new Map();
			struct_types.set(struct_name + " *", struct_value_types);

			while (true) {
				// Struct contents
				pos++;
				let name = get_token();

				if (name == "}") { // Struct end
					write("})" + struct_name + ";\n");
					break;
				}

				// val?: i32 -> val: i32
				name = strip_optional(name);

				pos++;
				// :
				let type = read_type();

				// type_t * -> struct type *
				if (type.endsWith("_t *") && type != "string_t *") {
					let type_short = strip(type, 4); // _t *
					if (type_short.endsWith("_array")) { // tex_format_t_array -> i32_array
						for (let e of enums) {
							if (type_short.startsWith(e)) {
								type_short = "i32_array";
								break;
							}
						}
					}
					type = "struct " + type_short + " *";
				}

				skip_until(";");

				write("\t" + join_type_name(type, name) + ";\n");
				struct_value_types.set(name, type);
			}

			write("\n");
			continue;
		}
	}
}

function write_array_types() {
	// Array structs (any_array_t -> scene_t_array_t)
	let array_structs = new Map();
	for (pos = 0; pos < tokens.length; ++pos) {
		let token = get_token();
		if (get_token(1) == "[") {
			let type = token;
			if (type == "string") {
				type = "string_t";
			}
			if (is_struct(type)) {
				if (!array_structs.has(type)) {
					let as = "typedef PACK(struct " + type + "_array{" +
						type + "**buffer;int length;int capacity;})" +
						type + "_array_t;";
					array_structs.set(type, as);
				}
			}
			pos += 2; // Skip "[]"
		}
	}

	for (let as of array_structs.values()) {
		write(as);
		write("\n");
	}
	write("\n");
}

function write_fn_declarations() {
	fn_declarations = new Map();
	fn_default_params = new Map();
	let last_fn_name = "";
	for (pos = 0; pos < tokens.length; ++pos) {
		let token = get_token();

		if (token == "function") {
			// Return type + name
			pos++;
			let fn_name = get_token();

			if (fn_name == "main") {
				fn_name = "_main";
			}

			let ret = function_return_type();

			if (fn_name == "(") { // Anonymous function
				fn_name = last_fn_name + "_1";
				pos--;
			}

			last_fn_name = fn_name;

			// Params
			let _param_pos = 0;
			let params = "(";
			pos++; // (
			while (true) {
				pos++;
				token = get_token(); // Param name, ) or ,

				if (token == ")") { // Params end
					break;
				}

				if (token == ",") { // Next param
					params += ",";
					_param_pos++;
					continue;
				}

				pos++; // :
				let type = read_type();
				params += join_type_name(type, token);

				// Store param default
				if (get_token(1) == "=") {
					let param = get_token(2);
					param = enum_access(param);
					fn_default_params.set(fn_name + _param_pos, param);
					pos += 2;
				}
			}
			params += ")";

			let fn_decl = ret + " " + fn_name + params;
			write(fn_decl + ";\n");

			fn_declarations.set(fn_name, fn_decl);
		}
	}
	write("\n");
}

function write_globals() {
	global_inits = [];
	global_ptrs = [];
	for (pos = 0; pos < tokens.length; ++pos) {
		let token = get_token();

		if (token == "let") {
			pos++;
			let name = get_token();

			pos++; // :
			let type = read_type();

			if (type != "f32" &&
				type != "i32" &&
				type != "u32" &&
				type != "i16" &&
				type != "u16" &&
				type != "i8" &&
				type != "u8" &&
				type != "bool" &&
				enums.indexOf(type) == -1) {
				global_ptrs.push(name);
			}

			write(join_type_name(type, name) + ";");

			// Init this var in _kickstart()
			let is_initialized = get_token(1) == "=";
			if (is_initialized) {
				let init = name;
				tabs = 1;
				while (true) {
					pos++;
					token = get_token();

					token = enum_access(token);
					token = array_access(token);
					token = struct_alloc(token);

					if (token == ";") {
						break;
					}

					init += get_filled_fn_params(token);

					// [] -> _array_create
					token = array_create(token, name);

					if (token == "map_create") {
						let t = value_type(name);
						if (t == "i32_map_t *") {
							token = "i32_map_create";
						}
						else {
							token = "any_map_create";
						}
					}

					init += token;
				}
				global_inits.push(init);
			}

			write("\n");
		}

		// Skip function blocks
		if (token == "function") {
			skip_until("{");
			skip_block();
		}
	}
}

function write_kickstart() {
	// Start function
	write("\nvoid _kickstart() {\n");
	// Init globals
	for (let val of global_inits) {
		write("\t");
		let name = val.split("=")[0];
		let global_alloc = global_ptrs.indexOf(name) > -1 && !val.endsWith("=null");
		if (global_alloc) {
			write("gc_free(" + name + ");");  // Make sure there are no other users?
		}
		write(val + ";");
		if (global_alloc) {
			write("gc_global(" + name + ");");
		}
		write("\n");
	}
	write("\t_main();\n");
	write("\tkinc_start();\n");
	write("}\n\n");
}

function write_function() {
	// Function declaration
	let fn_name = get_token();

	if (fn_name == "main") {
		fn_name = "_main";
	}

	let fn_decl = fn_declarations.get(fn_name);
	write(fn_decl + "{\n");

	// Function body
	// Re-set function param types into value_types map
	set_fn_param_types();
	skip_until("{");

	tabs = 1;
	new_line = true;
	let mark_global = null;
	let anon_fn = fn_name;
	let nested = false;

	while (true) {
		pos++;
		token = get_token();

		if (token == "function") { // Begin nested function
			anon_fn += "_1";
			write("&" + anon_fn);
			if (get_token(-1) == "=") {
				write(";\n");
			}

			skip_until("{");
			write = string_write;
			let fn_decl = fn_declarations.get(anon_fn);
			write(fn_decl + "{\n");
			nested = true;
			continue;
		}
		if (token == "}" && nested) { // End nested function
			nested = false;
			write("}\n\n");
			write = stream_write;
			continue;
		}

		// Function end
		if (token == "}" && tabs == 1) {
			write("}\n\n");
			break;
		}

		handle_tabs(token);

		if (token == "for") { // Skip new lines in for (;;) loop
			is_for_loop = true;
		}
		if (token == "{") {
			is_for_loop = false;
		}

		// Write type and name
		if (token == "let") {
			pos++;
			let name = get_token();
			pos++; // :
			let type = read_type();
			write(join_type_name(type, name));
			// = or ;
			pos++;
			token = get_token();
		}

		// Turn val.a into val_a or val->a
		token = enum_access(token);
		token = struct_access(token);

		// array[0] -> array->buffer[0]
		token = array_access(token);

		// Use static alloc for global pointers
		if (get_token(1) == "=" && token != ":" && get_token(2) != "null") {
			if (global_ptrs.indexOf(token) > -1) {
				mark_global = token;
				write("gc_free(" + mark_global + ");"); // Make sure there are no other users?
			}
		}
		if (token == ";" && mark_global != null) {
			write(";gc_global(" + mark_global + ")");
			mark_global = null;
		}

		// "= { ... }" -> GC_ALLOC_INIT
		token = struct_alloc(token);

		// [] -> _array_create
		token = array_create(token, get_token(-2));

		// array_push -> i32_array_push/any_array_push
		if (token == "array_push") {
			let value = get_token(2);

			if (get_token(3) == "[") { // raws[i].value
				value = get_token(6).substring(1); // .value
			}

			if (value.lastIndexOf(".") > -1) {
				value = value.substring(value.lastIndexOf(".") + 1, value.length);
			}
			let type = array_type(value);
			token = type + "_array_push";
		}

		// Maps
		token = map_ops(token);

		// Strings
		let is_string = token.startsWith("\"") || value_type(token) == "string_t *";

		if (!is_string) {
			let _pos = pos;
			read_piece();
			if (get_token(1) == "+" || get_token(1) == "+=" || get_token(1) == "==" || get_token(1) == "!=") {
				if (get_token(2).startsWith("\"") || value_type(get_token(2)) == "string_t *") {
					is_string = true;
				}
			}
			pos = _pos;
		}

		if (is_string && get_token(2) == "null") {
			is_string = false;
		}

		if (is_string) {
			token = string_ops(token);
		}

		// "str->length" -> "string_length(str)"
		token = string_length(token);

		if (token == "armpack_decode") {
			token = "armpack_decodeb"; // todo
		}

		// a / b - > a / (float)b
		if (token == "/") {
			token = "/(float)";
		}

		// a(1) -> a(1, 2) // a(x: i32, y: i32 = 2)
		let filled_fn_params = get_filled_fn_params(token);
		if (token == ")") {
			token = filled_fn_params + ")";
		}

		// Write token
		write(token);

		handle_spaces(token, ["return", "else"]);

		handle_new_line(token);
	}
}

function write_functions() {
	for (pos = 0; pos < tokens.length; ++pos) {
		let token = get_token();
		if (token == "function") {
			pos++;
			write_function();
		}

		// Write anonymous function body
		if (string != "") {
			write(string);
			string = "";
		}
	}
}

function write_c() {
	write('#include <krom.h>\n\n');
	write_enums();
	write_types();
	write_array_types();
	write_fn_declarations();
	write('#include <krom_api.h>\n\n');
	write_globals();
	write_kickstart();
	write_functions();
}

// ██╗  ██╗    ██╗     ██████╗    ██╗  ██╗    ███████╗    ████████╗     █████╗     ██████╗     ████████╗
// ██║ ██╔╝    ██║    ██╔════╝    ██║ ██╔╝    ██╔════╝    ╚══██╔══╝    ██╔══██╗    ██╔══██╗    ╚══██╔══╝
// █████╔╝     ██║    ██║         █████╔╝     ███████╗       ██║       ███████║    ██████╔╝       ██║
// ██╔═██╗     ██║    ██║         ██╔═██╗     ╚════██║       ██║       ██╔══██║    ██╔══██╗       ██║
// ██║  ██╗    ██║    ╚██████╗    ██║  ██╗    ███████║       ██║       ██║  ██║    ██║  ██║       ██║
// ╚═╝  ╚═╝    ╚═╝     ╚═════╝    ╚═╝  ╚═╝    ╚══════╝       ╚═╝       ╚═╝  ╚═╝    ╚═╝  ╚═╝       ╚═╝

function kickstart() {
	let fs = require("fs");
	if (flags.minits_source == null) {
		flags.minits_source = fs.readFileSync(flags.minits_input).toString();
	}

	tokens = parse();
	stream = fs.createWriteStream(flags.minits_output);
	write_c();
	stream.end();
}

kickstart();
