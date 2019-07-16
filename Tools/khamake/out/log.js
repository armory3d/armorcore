"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
let myInfo = function (text, newline) {
    if (newline) {
        console.log(text);
    }
    else {
        process.stdout.write(text);
    }
};
let myError = function (text, newline) {
    if (newline) {
        console.error(text);
    }
    else {
        process.stderr.write(text);
    }
};
function set(log) {
    myInfo = log.info;
    myError = log.error;
}
exports.set = set;
function silent() {
    myInfo = function () { };
    myError = function () { };
}
exports.silent = silent;
function info(text, newline = true) {
    myInfo(text, newline);
}
exports.info = info;
function error(text, newline = true) {
    myError(text, newline);
}
exports.error = error;
//# sourceMappingURL=log.js.map