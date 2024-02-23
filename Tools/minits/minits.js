
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

function is_alpha(code) {
	return (code > 64 && code < 91)  || // A-Z
		   (code > 96 && code < 123) || // a-z
		   (code == 95);				// _
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

let stream;
let tabs = 0;
let new_line = false;

function handle_tabs(token) {
	// Entering block, add tab
	if (token == "{") {
		tabs++;
	}
	else if (token == "}") {
		tabs--;
	}

	// New line, add tabs
	if (new_line) {
		for (let i = 0; i < tabs; ++i) {
			stream.write("\t");
		}
	}
}

function handle_new_line(token) {
	// Insert new line
	new_line = token == ";" || token == "{" || token == "}";
	if (new_line) {
		stream.write("\n");
	}
}

function handle_spaces(token, keywords) {
	// Add space to separate keywords
	for (let kw of keywords) {
		if (token == kw) {
			stream.write(" ");
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

function write_js() {
	let is_let_declaration = false;
	let is_func_declaration = false;

	for (pos = 0; pos < tokens.length; ++pos) {
		let token = get_token();

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
				pos++;
				token = get_token();

				// Skip Map "<a,b>" info
				if (token == "<") {
					while (true) {
						pos++;
						token = get_token();
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
							pos--;
							break;
					}
				}
				else if (is_func_declaration) {
					let next = get_token(1);
					if (token == "," ||
						(token == ")" &&
							next != "=>" && next != "[") || // ()=>, (()=>x)[]
						token == "{" ||
						token == "=") {
							pos--;
							break;
						}
				}
			}

			// Skip writing ":"
			continue;
		}

		// Skip "type = {};" and "type = x;" info
		if (token == "type" && get_token(2) == "=") {

			// "type = x;"
			if (get_token(4) == ";") {
				pos += 4;
				continue;
			}

			// "type = {};"
			while (true) {
				pos++;
				token = get_token();
				// type struct ending
				if (token == "}") {
					pos++;
					break;
				}
			}

			// Skip writing "type"
			continue;
		}

		// Skip (x as type) cast
		if (token == "as") {
			pos += 2;
			continue;
		}

		// Turn enum {} into let {} structure
		if (token == "enum") {
			stream.write("let " + get_token(1) + "={");
			pos += 2;

			let counter = 0;
			while (true) {
				pos++; // Item name
				token = get_token();
				stream.write(token);

				if (token == "}") { // Enum end
					stream.write(";\n");
					break;
				}

				pos++; // = or ,
				token = get_token();

				if (token == "=") {
					stream.write(":");

					pos++; // n
					token = get_token();
					stream.write(token);

					pos++; // ,
				}
				else { // Enum with no numbers
					stream.write(":" + counter);
					counter++;
				}

				stream.write(",");
			}

			continue;
		}

		handle_tabs(token);

		// Write token
		stream.write(token);

		handle_spaces(token, ["let", "else", "function", "return", "typeof"]);

		handle_new_line(token);
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

function skip_until(s) {
	while (true) {
		let token = get_token();
		if (token == s) {
			break;
		}
		pos++;
	}
}

function skip_block() { // {}
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
	// Continue until "{" is found
	skip_until("{");
	pos -= 2; // ): i32 {
	let result = get_token() == ":" ? read_type() : "void";
	pos = _pos;
	return result;
}

function join_type_name(type, name) {
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

		params += ")";
		skip_until("=>");
		let ret = read_type();
		type = ret + "(*NAME)" + params;
	}

	if (get_token(1) == "[") { // Array
		pos++; // Skip "["
		pos++; // Skip "]"
	}

	if (type == "string") {
		type = "string_t";
	}

	if (is_struct(type)) {
		type += " *";
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
	if (s.indexOf(".") > -1 && is_alpha(s.charCodeAt(0))) {
		s = s.replaceAll(".", "->");
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

function strip_t_star(type) {
	return type.substring(0, type.length - 4); // Strip "_t *"
}

function strip_optional(name) {
	if (name.endsWith("?")) {
		name = name.substring(0, name.length - 1); // Strip "?" in "val?"
	}
	return name;
}

function write_c() {
	stream.write('#include <krom.h>\n\n');

	// Enums
	enums = [];
	for (pos = 0; pos < tokens.length; ++pos) {
		let token = get_token();

		// Turn "enum name {}" into "typedef enum {} name;"
		if (token == "enum") {
			pos++;
			let enum_name = get_token();
			enums.push(enum_name);

			stream.write("typedef enum{\n");
			pos++; // {

			while (true) {
				// Enum contents
				pos++;
				token = get_token(); // Item name

				if (token == "}") { // Enum end
					stream.write("}" + enum_name + ";\n");
					break;
				}

				stream.write("\t" + enum_name + "_" + token);

				pos++; // = or ,
				token = get_token();

				if (token == "=") { // Enum value
					pos++; // n
					token = get_token();
					stream.write("=" + token);
					pos++; // ,
				}

				stream.write(",\n");
			}

			stream.write("\n");
			continue;
		}
	}

	// Types
	for (pos = 0; pos < tokens.length; ++pos) {
		let token = get_token();

		// Turn "type x = {};" into "typedef struct {} x;"
		// Turn "type x = y;" into "typedef x y;"
		if (token == "type") {
			pos++;
			let struct_name = get_token();
			let stuct_name_short = strip_t(struct_name);

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

				stream.write("typedef " + type + " " + struct_name + ";\n\n");
				skip_until(";");
				continue;
			}

			// "type x = {};"
			stream.write("typedef struct " + stuct_name_short + "{\n");

			while (true) {
				// Struct contents
				pos++;
				let name = get_token();

				if (name == "}") { // Struct end
					stream.write("}" + struct_name + ";\n");
					break;
				}

				// val?: i32 -> val: i32
				name = strip_optional(name);

				pos++;
				// :
				let type = read_type();

				// type_t * -> struct type *
				if (type.endsWith("_t *")) {
					let token_type_short = strip_t_star(type);
					type = "struct " + token_type_short + " *";
				}

				skip_until(";");

				stream.write("\t" + join_type_name(type, name) + ";\n");
			}

			stream.write("\n");
			continue;
		}
	}

	// Function declarations
	let fn_declarations = new Map();
	let fn_default_params = new Map();
	for (pos = 0; pos < tokens.length; ++pos) {
		let token = get_token();

		if (token == "function") {
			// Return type + name
			pos++;
			let fn_name = get_token();
			let ret = function_return_type();

			// Params
			let param_pos = 0;
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
					param_pos++;
					continue;
				}

				let name = token;
				pos++; // :

				let type = read_type();

				params += join_type_name(type, name);

				// Store param default
				if (get_token(1) == "=") {
					fn_default_params.set(fn_name + param_pos, get_token(2));
					pos += 2;
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
	for (pos = 0; pos < tokens.length; ++pos) {
		let token = get_token();

		if (token == "let") {
			pos++;
			let name = get_token();

			pos++; // :
			let type = read_type();

			stream.write(join_type_name(type, name) + ";");

			// Init this var in _globals_init()
			let is_initialized = get_token(1) == "=";
			if (is_initialized) {
				let init = name;
				let fn_call = "";
				let param_pos = 0;
				while (true) {
					pos++;
					token = get_token();

					token = enum_access(token);

					if (token == ";") {
						break;
					}

					if (token == "(") {
						// Function is being called to init this variable
						fn_call = get_token(-1);
					}

					if (token == ",") {
						// Param has already beed passed manually
						param_pos++;
					}

					if (token == ")") {
						// Fill in default parameters if needed
						if (get_token(-1) != "(") {
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
				global_inits.push(init);
			}

			stream.write("\n");
		}

		// Skip function blocks
		if (token == "function") {
			skip_until("{");
			skip_block();
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
	for (pos = 0; pos < tokens.length; ++pos) {
		let token = get_token();

		if (token == "function") {
			// Function declaration
			pos++;
			let fn_name = get_token();
			let fn_decl = fn_declarations.get(fn_name);
			stream.write(fn_decl + "{\n");

			// Function body
			skip_until("{");
			tabs = 1;
			new_line = true;
			malloc_type = "";

			while (true) {
				pos++;
				token = get_token();

				// Function end
				if (token == "}" && tabs == 1) {
					stream.write("}\n\n");
					break;
				}

				handle_tabs(token);

				// Write type and name
				if (token == "let") {
					pos++;
					let name = get_token();

					pos++; // :
					let type = read_type();
					malloc_type = type;

					stream.write(join_type_name(type, name));

					// = or ;
					pos++;
					token = get_token();
				}

				// Turn val.a into val_a or val->a
				token = enum_access(token);
				token = struct_access(token);

				// Turn "= {}" into malloc()
				if (get_token(-1) == "=" && token == "{" && get_token(1) == "}") {
					if (malloc_type.endsWith(" *")) { // mystruct * -> mystruct
						malloc_type = malloc_type.substring(0, malloc_type.length - 2);
					}
					token = "malloc(sizeof(" + malloc_type + "))"
					pos++; // }
					tabs--;
				}

				// Write token
				stream.write(token);

				handle_spaces(token, ["return", "else"]);

				handle_new_line(token);
			}
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
	stream = fs.createWriteStream(flags.minits_output);

	if (flags.minits_output.endsWith(".js")) {
		write_js();
	}
	else if (flags.minits_output.endsWith(".c")) {
		write_c();
	}

	stream.close();
}

kickstart();
