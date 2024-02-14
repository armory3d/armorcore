
// minits transpiler
// ../../Kinc/make --kfile minits.js
// js:
// ../../Kinc/make --kfile test.js
// c:
// ../../Kinc/make --run

const fs = require("fs");

let flags = globalThis.flags;
if (flags == null) {
	flags.minits_input = "./test.ts";
	flags.minits_output = "./test.c";
}

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

// let js_footer = `\nkickstart();\n`;
let js_footer = `\n`;

let c_header = `
#include <kinc/log.h>
#include <iron/iron_string.h>
#include <iron/iron_array.h>
#define krom_log(str) kinc_log(KINC_LOG_LEVEL_INFO, "%f", str)
#define i32 int32_t
#define f32 float
#define u8 uint8_t
#define ARGV char *
`;

// Parsing
let specials = [":", ";", ",", "(", ")", "[", "]", "{", "}", "<", ">", "!"];
let is_comment = false;
let is_string = false;
let is_func_declaration = false;
let new_line = false;
let tabs = 0;
let pos = 0;
let file = "";
let tokens = [];

// Writing
let is_for_loop_header = false;

// Parsing
function is_alpha_numeric(code) {
	return (code > 47 && code < 58) || // 0-9
		   (code == 45) 			|| // -
		   (code == 43) 			|| // +
		   (code == 95) 			|| // _
		   (code == 46) 			|| // .
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
		if (token == null) { // Throw away this token
			continue;
		}
		if (token == "") { // No more tokens
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

function write_js(stream) {
	let is_let_declaration = false;

	stream.write(js_header);

	for (let i = 0; i < tokens.length; ++i) {
		let token = get_token(i);
		let next = get_token(i + 1);
		let next_next = get_token(i + 2);

		// Skip comment tokens
		if (token.startsWith("//")) {
			continue;
		}

		if (token == "let") {
			is_let_declaration = true;
		}
		else if (token == "function") {
			is_func_declaration = true;
		}

		if (is_let_declaration && (token == "=" || token == ";")) {
			is_let_declaration = false;
		}
		else if (is_func_declaration && token == "{") {
			is_func_declaration = false;
		}

		let is_declaration = is_let_declaration || is_func_declaration;

		// Skip ": type" info
		if (token == ":" && is_declaration) {
			while (true) {
				i++;
				let token = get_token(i);
				let next = get_token(i + 1);

				// Skip Map "<a,b>" info
				if (token == "<") {
					while (true) {
						i++;
						let token = get_token(i);
						// Map block ending
						if (token == ">") {
							break;
						}
					}
					continue;
				}

				// Type token is over
				if (is_let_declaration) {
					if (token == "=" ||
						token == ";") {
							i--;
							break;
					}
				}
				else if (is_func_declaration) {
					if (token == "," ||
						(token == ")" &&
							next != "=>" && next != "[") || // ()=>, (()=>x)[]
						token == "{" ||
						token == "=") {
							i--;
							break;
						}
				}
			}

			// Skip writing ":"
			continue;
		}

		// Skip "type = {};" and "type = x;" info
		if (token == "type" && next_next == "=") {

			// "type = x;"
			let next_next_next_next = get_token(i + 4);
			if (next_next_next_next == ";") {
				i += 4;
				continue;
			}

			// "type = {};"
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

		// Turn enum {} into let {} structure
		if (token == "enum") {
			stream.write("let " + next + "={");
			i++;
			i++;

			let counter = 0;
			while (true) {
				i++; // Item name
				let token = get_token(i);
				stream.write(token);

				if (token == "}") { // Enum end
					stream.write(";");
					stream.write("\n");
					break;
				}

				i++; // = or ,
				token = get_token(i);

				if (token == "=") {
					stream.write(":");

					i++; // n
					token = get_token(i);
					stream.write(token);

					i++; // ,
				}
				else { // Enum with no numbers
					stream.write(":");

					stream.write(counter + "");
					counter++;
				}

				stream.write(",");
			}

			continue;
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
		if (new_line) {
			for (let i = 0; i < tabs; ++i) {
				stream.write("\t");
			}
		}

		// Write token
		stream.write(token);

		// Add space to separate keywords
		if (token == "let" ||
			(token == "else" && next != "{") || // else if
			token == "function" ||
			// token == "type" ||
			token == "new" || //// tmp
			token == "of" || //// tmp
			token == "in" || //// tmp
			token == "typeof" || //// tmp
			next == "of" || //// tmp
			next == "in" || //// tmp
			token == "return") {
			stream.write(" ");
		}

		// Insert new line after each ";", but skip "for (;;)"
		new_line = token == ";" && !is_for_loop_header;

		// Insert new line after each "{", but skip "{}"
		if (token == "{" && next != "}") {
			new_line = true;
		}

		// Insert new line after each "}", but skip "};"
		if (token == "}" && next != ";") {
			new_line = true;
		}

		if (new_line) {
			stream.write("\n");
		}
	}

	stream.write(js_footer);
}

function c_get_function_return_type(i) {
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

function write_c(stream) {
	let is_struct = false;
	let struct_name = "";
	let struct_list = []; // Pass with &
	let pointer_list = []; // Access with ->

	stream.write(c_header);

	for (let i = 0; i < tokens.length; ++i) {
		let token = get_token(i);
		let next = get_token(i + 1);
		let next_next = get_token(i + 2);
		let next_next_next = get_token(i + 3);

		// Skip comment tokens
		if (token.startsWith("//")) {
			continue;
		}

		// Turn "type = {}" into "typedef struct {}"
		if (token == "type") {
			token = "typedef struct";
			stream.write(token);

			// Read type name
			i++;
			token = get_token(i);
			// Strip _t
			struct_name = token.substring(0, token.length - 2);
			// Write struct name
			stream.write(" " + struct_name);

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
				stream.write("}" + struct_name + "_t;\n");

				i++; // "}" parsed
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
		if (new_line) {
			for (let i = 0; i < tabs; ++i) {
				stream.write("\t");
			}
			new_line = false;
		}

		// Function declaration start
		if (token == "function") {
			is_func_declaration = true;

			// Replace "function" with actual function return type
			token = c_get_function_return_type(i) + " ";

			// Turn string type into string_t
			if (token == "string ") {
				token = "string_t ";
			}
		}

		// Function declaration end
		if (token == ")") {
			is_func_declaration = false;
		}

		// Skip function return type, as it's already written
		if (is_func_declaration && token == ")" && next == ":") {
			stream.write(token);
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
				stream.write("array_");
			}

			// Turn string type into string_t
			if (next_next == "string") {
				next_next = "string_t";
			}

			// Turn any type into void *
			if (next_next == "any") {
				next_next = "void *";
			}

			// This variable is a structure
			if (next_next != "i32" &&
				next_next != "f32" &&
				next_next != "u8") {
				struct_list.push(token);
			}

			// Write variable type
			stream.write(next_next);

			if (is_array) {
				// Map arrays to iron arrays
				stream.write("_t");
			}

			stream.write(" ");

			// Structure passed as a param, take a pointer
			if (is_func_declaration &&
				struct_list.indexOf(token) > -1) {
				stream.write("*");
				pointer_list.push(token);
			}

			// val?: i32
			if (token.endsWith("?")) {
				token = token.substring(0, token.length - 1);
			}

			// Write variable name
			stream.write(token);

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
			stream.write("{}");
			i++; // ]
			continue;
		}

		// Skip writing let
		if (token == "let") {
			continue;
		}

		// Structure passed into a function, add &
		if (struct_list.indexOf(token) > -1) {
			// if (!is_func_declaration) {
				stream.write("&");
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
		stream.write(token);

		// Add space to separate keywords
		let space =
			token == "return";
		if (space) {
			stream.write(" ");
		}

		// Insert new line after each ";", but skip "for (;;)"
		new_line = token == ";" && !is_for_loop_header;

		// Insert new line after each "{", but skip "{}"
		if (token == "{" && next != "}") {
			new_line = true;
		}

		// Insert new line after each "}", but skip "};"
		if (token == "}" && next != ";") {
			new_line = true;
		}

		if (new_line) {
			stream.write("\n");
		}
	}
}

function kickstart() {
	if (!fs.existsSync(flags.minits_input)) {
		return;
	}

	file = fs.readFileSync(flags.minits_input).toString();
	tokens = parse(file);

	let stream = fs.createWriteStream(flags.minits_output);

	if (flags.minits_output.endsWith(".js")) {
		write_js(stream);
	}
	else if (flags.minits_output.endsWith(".c")) {
		write_c(stream);
	}

	stream.close();
}

kickstart();
