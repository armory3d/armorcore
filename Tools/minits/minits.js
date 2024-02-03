
// minits transpiler
// ../../Kinc/make --kfile minits.js
// js:
// ../../Kinc/make --kfile main.js
// c:
// ../../Kinc/make --run

const fs = require("fs");

let flags = {};
// flags.target = "js";
flags.target = "c";
flags.remove_comments = true;
flags.js_remove_types = true;
flags.input = "./main.ts";
// flags.output = "./main.js";
flags.output = "./main.c";

// let ts_header = `
// declare type i8 = number;
// declare type i16 = number;
// declare type i32 = number;
// declare type u8 = number;
// declare type u16 = number;
// declare type u32 = number;
// declare type f32 = number;
// declare type f64 = number;
// declare type bool = boolean;
// declare type ARGV = string[];
// declare type array_f32_t = [];
// `;

let js_header = `
function krom_log(s) { console.log(s); }
function array_f32_push(ar, e) { ar.push(e); }
`;

let js_footer = `\nkickstart();\n`;

let c_header = `
#include <kinc/log.h>
#include <iron/iron_string.h>
#include <iron/iron_array.h>
#define krom_log(str) kinc_log(KINC_LOG_LEVEL_INFO, "%f", str)
#define i32 int
#define f32 float
#define ARGV char *
`;

// Parsing
let specials = [":", ";", ",", "(", ")", "[", "]", "{", "}"];
let is_comment = false;
let is_string = false;
let pos = 0;
let file = "";
let tokens = [];

// Writing
let is_for_loop_header = false;

// Parsing
function is_alpha_numeric(code) {
	return (code > 47 && code < 58) || // 0-9
		   (code > 64 && code < 91) || // A-Z
		   (code > 96 && code < 123);  // a-z
}

function is_white_space(code) {
	return code == 32 || // " "
		   code == 9  || // "\t"
		   code == 10;   // "\n"
}

function read_token() {
	// Skip white space
	while (is_white_space(file.charCodeAt(pos))) {
		pos++;
	}

	let token = "";

	while (pos < file.length) {

		let c = file.charAt(pos);

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
				break;
			}

			// Prevent parsing of comments
			continue;
		}

		// String start / end
		if (c == "\"") {
			is_string = !is_string;
		}

		if (is_string) {
			token += c;
			pos++;

			// Prevent parsing of strings
			continue;
		}

		// Token end
		if (is_white_space(file.charCodeAt(pos))) {
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

	while (true) {
		let token = read_token();
		if (token == "") {
			break;
		}
		tokens.push(token);
	}

	return tokens;
}

// Writing
function get_token(i) {
	if (i < tokens.length) {
		return tokens[i];
	}
	return "";
}

function write_js() {
	let out = "";
	let tabs = 0;

	for (let i = 0; i < tokens.length; ++i) {
		let token = get_token(i);
		let next = get_token(i + 1);

		// Skip comment tokens
		if (flags.remove_comments) {
			if (token.startsWith("//")) {
				continue;
			}
		}

		// Skip "type = {}" and ": type" info
		if (flags.js_remove_types) {
			if (token == ":") {
				while (true) {
					i++;
					let token = get_token(i);

					// Type token is over
					if (token == "=" ||
						token == ";" ||
						token == ")" ||
						token == "{") {
						i--;
						break;
					}
				}

				// Skip writing ":"
				continue;
			}

			if (token == "type") {
				while (true) {
					i++;
					let token = get_token(i);
					// type struct ending
					if (token == "}") {
						i++;
						break;
					}
				}

				// Skip writing "type"
				continue;
			}
		}

		// Entering for loop header
		if (token == "for") {
			is_for_loop_header = true;
		}

		// For loop header end
		if (is_for_loop_header && token == "{") {
			is_for_loop_header = false;
		}

		// Entering block, add tab
		if (token == "{") {
			tabs += 1;
		}
		else if (token == "}") {
			tabs -= 1;
		}

		// New line, add tabs
		if (out.charAt(out.length - 1) == "\n") {
			for (let i = 0; i < tabs; ++i) {
				out += "\t";
			}
		}

		// Write token
		out += token;

		// Add space to separate keywords
		let space =
			token == "let" ||
			token == "function" ||
			token == "return" ||
			token == "type";
		if (space) {
			out += " ";
		}

		// Insert new line after each ";", but skip "for (;;)"
		let new_line = token == ";" && !is_for_loop_header;

		// Insert new line after each "{", but skip "{}"
		if (token == "{" && next != "}") {
			new_line = true;
		}

		// Insert new line after each "}", but skip "};"
		if (token == "}" && next != ";") {
			new_line = true;
		}

		if (new_line) {
			out += "\n";
		}
	}

	fs.writeFileSync(flags.output, js_header + out + js_footer);
}

function get_function_return_type(i) {
	while (true) {
		// Continue until ")" is found
		if (get_token(i) == ")") {
			// Type specified
			if (get_token(i + 1) == ":") {
				return get_token(i + 2);
			}
			// Type not specified
			else {
				return "void";
			}
		}
		i++;
	}
}

function write_c() {
	let out = "";
	let tabs = 0;

	let is_function_header = false;
	let is_struct = false;
	let struct_name = "";
	let struct_list = []; // Pass with &
	let pointer_list = []; // Access with ->

	for (let i = 0; i < tokens.length; ++i) {
		let token = get_token(i);
		let next = get_token(i + 1);
		let next_next = get_token(i + 2);
		let next_next_next = get_token(i + 3);

		// Skip comment tokens
		if (flags.remove_comments) {
			if (token.startsWith("//")) {
				continue;
			}
		}

		// Turn "type = {}" into "typedef struct {}"
		if (token == "type") {
			token = "typedef struct";
			out += token;

			// Read type name
			i++;
			token = get_token(i);
			// Strip _t
			struct_name = token.substring(0, token.length - 2);
			// Write struct name
			out += " " + struct_name;

			// Skip =
			i++;

			// Inside a struct now
			is_struct = true;

			continue;
		}

		if (is_struct) {
			// Struct end
			if (token == "}") {
				is_struct = false;

				// Append struct name after "}"
				out += "}" + struct_name + "_t;\n";

				// "}" parsed
				i++;
				tabs--;
				continue;
			}
		}

		// Entering for loop header
		if (token == "for") {
			is_for_loop_header = true;
		}

		// For loop header end
		if (is_for_loop_header && token == "{") {
			is_for_loop_header = false;
		}

		// Entering block, add tab
		if (token == "{") {
			tabs += 1;
		}
		else if (token == "}") {
			tabs -= 1;
		}

		// New line, add tabs
		if (out.charAt(out.length - 1) == "\n") {
			for (let i = 0; i < tabs; ++i) {
				out += "\t";
			}
		}

		// Function declaration start
		if (token == "function") {
			is_function_header = true;

			// Replace "function" with actual function return type
			token = get_function_return_type(i) + " ";
		}

		// Function declaration end
		if (token == ")") {
			is_function_header = false;
		}

		// Skip function return type, as it's already written
		if (token == ")" && next == ":") {
			out += token;
			i++; // ":" token
			i++; // "i32" token
			continue;
		}

		// Variable type
		if (next == ":") {

			let is_array = false;
			if (next_next_next == "[") {
				is_array = true;
				// Map arrays to iron arrays
				out += "array_";
			}

			// Turn string type into string_t
			if (next_next == "string") {
				next_next = "string_t";
			}

			// This variable is a structure
			if (next_next != "i32" &&
				next_next != "f32") {
				struct_list.push(token);
			}

			// Write variable type
			out += next_next;

			if (is_array) {
				// Map arrays to iron arrays
				out += "_t";
			}

			out += " ";

			// Structure passed as a param, take a pointer
			if (is_function_header &&
				struct_list.indexOf(token) > -1) {
				out += "*";
				pointer_list.push(token);
			}

			// Write variable name
			out += token;

			i++; // :
			i++; // i32
			if (is_array) {
				i++; // [
				i++; // ]
			}
			continue;
		}

		// Turn ar = [] into ar = {}
		if (token == "[" && next == "]") {
			out += "{}";
			i++; // ]
			continue;
		}

		// Skip writing let
		if (token == "let") {
			continue;
		}

		// Structure passed into a function, add &
		if (struct_list.indexOf(token) > -1) {
			// if (!is_function_header) {
				out += "&";
			// }
		}

		// Structure member access
		if (token.indexOf(".") > -1) {
			let base = token.substring(0, token.indexOf("."));
			// It's a pointer, replace . with ->
			if (pointer_list.indexOf(base) > -1) {
				token = token.replace(".", "->");
			}
		}

		// Write token
		out += token;

		// Add space to separate keywords
		let space =
			token == "return";
		if (space) {
			out += " ";
		}

		// Insert new line after each ";", but skip "for (;;)"
		let new_line = token == ";" && !is_for_loop_header;

		// Insert new line after each "{", but skip "{}"
		if (token == "{" && next != "}") {
			new_line = true;
		}

		// Insert new line after each "}", but skip "};"
		if (token == "}" && next != ";") {
			new_line = true;
		}

		if (new_line) {
			out += "\n";
		}
	}

	fs.writeFileSync(flags.output, c_header + out);
}

function kickstart() {
	if (!fs.existsSync(flags.input)) {
		return;
	}

	file = fs.readFileSync(flags.input).toString();
	tokens = parse(file);

	if (flags.target == "js") {
		write_js();
	}
	else if (flags.target == "c") {
		write_c();
	}
}

kickstart();
