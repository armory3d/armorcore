
// minits transpiler
// ../../Kinc/make --kfile minits.js
// js:
// ../../Kinc/make --kfile test.js
// c:
// ../../Kinc/make --run

let flags = globalThis.flags;
if (flags == null) {
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
let file = "";
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
	pos = 0;

	while (true) {
		let token = read_token();
		if (token == null) { // Throw away this token
			continue;
		}
		if (token.startsWith("//")) { // Throw away comment tokens
			continue;
		}
		if (token == "") { // No more tokens
			break;
		}
		tokens.push(token);
	}

	return tokens;
}

// ██╗    ██╗    ██████╗     ██╗    ████████╗    ███████╗             ██╗    ███████╗
// ██║    ██║    ██╔══██╗    ██║    ╚══██╔══╝    ██╔════╝             ██║    ██╔════╝
// ██║ █╗ ██║    ██████╔╝    ██║       ██║       █████╗               ██║    ███████╗
// ██║███╗██║    ██╔══██╗    ██║       ██║       ██╔══╝          ██   ██║    ╚════██║
// ╚███╔███╔╝    ██║  ██║    ██║       ██║       ███████╗        ╚█████╔╝    ███████║
//  ╚══╝╚══╝     ╚═╝  ╚═╝    ╚═╝       ╚═╝       ╚══════╝         ╚════╝     ╚══════╝

let tabs = 0;
let new_line = false;
let is_for_loop_header = false;
let is_func_declaration = false;

function get_token(i) {
	if (i < tokens.length) {
		return tokens[i];
	}
	return "";
}

function write_js(stream) {
	let is_let_declaration = false;

	for (let i = 0; i < tokens.length; ++i) {
		let token = get_token(i);
		let next = get_token(i + 1);
		let next_next = get_token(i + 2);

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

		// Skip (x as type) cast
		if (token == "as") {
			i += 2;
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
			token == "typeof" || //// tmp
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

	stream.write("start();");
	stream.write("\n");
}

// ██╗    ██╗    ██████╗     ██╗    ████████╗    ███████╗         ██████╗
// ██║    ██║    ██╔══██╗    ██║    ╚══██╔══╝    ██╔════╝        ██╔════╝
// ██║ █╗ ██║    ██████╔╝    ██║       ██║       █████╗          ██║
// ██║███╗██║    ██╔══██╗    ██║       ██║       ██╔══╝          ██║
// ╚███╔███╔╝    ██║  ██║    ██║       ██║       ███████╗        ╚██████╗
//  ╚══╝╚══╝     ╚═╝  ╚═╝    ╚═╝       ╚═╝       ╚══════╝         ╚═════╝

let enums = [];

function skip_until(pos, s) {
	let i = 0;
	while (true) {
		let token = get_token(pos + i);
		if (token == s) {
			break;
		}
		i++;
	}
	return i;
}

function skip_block(pos) { // {}
	let i = 0;
	i++; // {
	let nested = 1;
	while (true) {
		let token = get_token(pos + i);
		if (token == "}") {
			nested--;
			if (nested == 0) {
				break;
			}
		}
		else if (token == "{") {
			nested++;
		}
		i++;
	}
	return i;
}

function function_return_type(i) {
	// Continue until "{" is found
	i += skip_until(i, "{");
	i -= 2; // ): i32 {
	// Type specified
	if (get_token(i) == ":") {
		return get_token(i + 1);
	}
	// Type not specified
	else {
		return "void";
	}
}

function to_c_type(s) {
	if (s == "string") {
		return "string_t";
	}
	if (s == "(") {
		// todo: ()=>void
		return "any";
	}
	return s;
}

function to_c_enum(s) {
	// Turn enum_t.VALUE into enum_t_VALUE
	for (let e of enums) {
		if (s.indexOf(e + ".") > -1) {
			s = s.replaceAll(e + ".", e + "_");
		}
	}
	return s;
}

function is_struct(type) {
	// Ends with _t and is not an enum
	return type.endsWith("_t") && enums.indexOf(type) == -1;
}

function strip_t(type) {
	return type.substring(0, type.length - 2); // Strip "_t"
}

function strip_optional(name) {
	if (name.endsWith("?")) {
		name = name.substring(0, name.length - 1); // Strip "?" in "val?"
	}
	return name;
}

function write_c(stream) {
	stream.write('#include <krom.h>\n\n');

	// Enums
	enums = [];
	for (let i = 0; i < tokens.length; ++i) {
		let token = get_token(i);

		// Turn "enum name {}" into "typedef enum {} name;"
		if (token == "enum") {
			i++;
			let enum_name = get_token(i);
			enums.push(enum_name);

			stream.write("typedef enum{\n");
			i++; // {

			while (true) {
				// Enum contents
				i++;
				let token = get_token(i); // Item name

				if (token == "}") { // Enum end
					stream.write("}" + enum_name + ";\n");
					break;
				}

				stream.write("\t" + enum_name + "_" + token);

				i++; // = or ,
				token = get_token(i);

				if (token == "=") { // Enum value
					i++; // n
					token = get_token(i);
					stream.write("=" + token);
					i++; // ,
				}

				stream.write(",\n");
			}

			stream.write("\n");
			continue;
		}
	}

	// Types
	for (let i = 0; i < tokens.length; ++i) {
		let token = get_token(i);

		// Turn "type x = {};" into "typedef struct {} x;"
		// Turn "type x = y;" into "typedef x y;"
		if (token == "type") {
			i++;
			let struct_name = get_token(i);
			let stuct_name_short = strip_t(struct_name);

			i++;
			token = get_token(i); // =
			if (token != "=") {
				continue;
			}

			i++;
			token = get_token(i);

			// "type x = y;"
			if (token != "{") {
				token = to_c_type(token);

				stream.write("typedef " + token + " " + struct_name + ";\n\n");
				i += skip_until(i, ";");
				continue;
			}

			// "type x = {};"
			stream.write("typedef struct " + stuct_name_short + "{\n");

			while (true) {
				// Struct contents
				i++;
				let token_name = get_token(i); // name

				if (token_name == "}") { // Struct end
					stream.write("}" + struct_name + ";\n");
					break;
				}

				// val?: i32 -> val: i32
				token_name = strip_optional(token_name);

				i++;
				// :

				i++;
				token_type = get_token(i); // type
				token_type = to_c_type(token_type);

				// type_t -> struct type *
				if (is_struct(token_type)) {
					let token_type_short = strip_t(token_type);
					token_type = "struct " + token_type_short + " *";
				}

				// todo: type_t[] -> array_t

				i += skip_until(i, ";");

				stream.write("\t" + token_type + " " + token_name + ";\n");
			}

			stream.write("\n");
			continue;
		}
	}

	// Function declarations
	let fn_declarations = new Map();
	let fn_default_params = new Map();
	for (let i = 0; i < tokens.length; ++i) {
		let token = get_token(i);

		if (token == "function") {
			// Return type + name
			i++;
			let fn_name = get_token(i);

			let ret = function_return_type(i);
			ret = to_c_type(ret);

			if (is_struct(ret)) {
				ret += " *";
			}

			// Params
			let param_pos = 0;
			let params = "(";
			i++; // (
			while (true) {
				i++;
				token = get_token(i);

				if (token == ")") { // Params end
					break;
				}

				if (token == ",") { // Next param
					params += ",";
					param_pos++;
					continue;
				}

				let name = token;
				i++; // :
				i++;
				let type = get_token(i);
				type = to_c_type(type);

				if (is_struct(type)) {
					type += " *";
				}

				params += type + " " + name;

				// Store param default
				if (get_token(i + 1) == "=") {
					fn_default_params.set(fn_name + param_pos, get_token(i + 2));
				}

				// Skip until , or )
				let nested = 0;
				while (true) {
					i++;
					token = get_token(i);
					if (token == "(") {
						nested++;
					}
					else if (token == "," && nested == 0) {
						i--;
						break
					}
					else if (token == ")") {
						if (nested == 0) {
							i--;
							break;
						}
						nested--;
					}
				}
			}
			params += ")";

			let fn_decl = ret + " " + fn_name + params;
			stream.write(fn_decl + ";\n");

			fn_declarations.set(fn_name, fn_decl);
		}
	}

	stream.write("\n");

	// Globals
	let global_inits = [];
	for (let i = 0; i < tokens.length; ++i) {
		let token = get_token(i);

		if (token == "let") {
			i += 1;
			let name = get_token(i); // var name

			i++; // :

			i++;
			let type = get_token(i); // var type
			type = to_c_type(type);

			if (is_struct(type)) {
				type += " *";
			}

			stream.write(type + " " + name + ";");

			// Init this var in _globals_init()
			let is_initialized = get_token(i + 1) == "=";
			if (is_initialized) {
				let init = name;
				let fn_call = "";
				let param_pos = 0;
				while (true) {
					i++;
					let token = get_token(i);

					if (token == ";") {
						break;
					}

					if (token == "(") {
						// Function is being called to init this variable
						fn_call = get_token(i - 1);
					}

					if (token == ",") {
						// Param has beed passed manually
						param_pos++;
					}

					if (token == ")") {
						// Fill in default parameters if needed
						if (get_token(i - 1) != "(") {
							// Param has beed passed manually
							param_pos++;
						}

						// If default param exists, fill it
						while (fn_default_params.has(fn_call + param_pos)) {
							if (param_pos > 0) {
								init += ",";
							}
							init += fn_default_params.get(fn_call + param_pos);
							param_pos++;
						}
					}

					init += token;
				}
				init = to_c_enum(init);
				global_inits.push(init);
			}

			stream.write("\n");
		}

		if (token == "function") {
			i += skip_until(i, "{");
			i += skip_block(i);
		}
	}

	// Globals init
	stream.write("\nvoid _globals_init() {\n");
	for (val of global_inits) {
		stream.write("\t");
		stream.write(val);
		stream.write(";\n");
	}
	stream.write("}\n\n");

	// Functions
	for (let i = 0; i < tokens.length; ++i) {
		let token = get_token(i);

		if (token == "function") {
			i++;
			let fn_name = get_token(i);
			let fn_decl = fn_declarations.get(fn_name);
			stream.write(fn_decl + "{\n");

			// Function body
			i += skip_until(i, "{");

			// stream.write("\t");

			// while (true) {
			// 	i++;
			// 	let token = get_token(i);

			// 	if (token == "}") {
			// 		stream.write("\n");
			// 		break;
			// 	}

			// 	stream.write(token);

			// 	if (token == ";") {
			// 		stream.write("\n\t");
			// 	}
			// }

			stream.write("}\n\n");
		}
	}
}

// ██╗  ██╗    ██╗     ██████╗    ██╗  ██╗    ███████╗    ████████╗     █████╗     ██████╗     ████████╗
// ██║ ██╔╝    ██║    ██╔════╝    ██║ ██╔╝    ██╔════╝    ╚══██╔══╝    ██╔══██╗    ██╔══██╗    ╚══██╔══╝
// █████╔╝     ██║    ██║         █████╔╝     ███████╗       ██║       ███████║    ██████╔╝       ██║
// ██╔═██╗     ██║    ██║         ██╔═██╗     ╚════██║       ██║       ██╔══██║    ██╔══██╗       ██║
// ██║  ██╗    ██║    ╚██████╗    ██║  ██╗    ███████║       ██║       ██║  ██║    ██║  ██║       ██║
// ╚═╝  ╚═╝    ╚═╝     ╚═════╝    ╚═╝  ╚═╝    ╚══════╝       ╚═╝       ╚═╝  ╚═╝    ╚═╝  ╚═╝       ╚═╝

function kickstart() {
	let fs = require("fs");
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
