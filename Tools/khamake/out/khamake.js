"use strict";
// Called from entry point, e.g. Kha/make.js
// This is where options are processed:
// e.g. '-t html5 --server'
Object.defineProperty(exports, "__esModule", { value: true });
const os = require("os");
const path = require("path");
const GraphicsApi_1 = require("./GraphicsApi");
const Options_1 = require("./Options");
let options = [
    {
        full: 'from',
        value: true,
        description: 'Location of your project',
        default: '.'
    },
    {
        full: 'to',
        value: true,
        description: 'Build location',
        default: 'build'
    },
    {
        full: 'graphics',
        short: 'g',
        description: 'Graphics api to use. Possible parameters are direct3d9, direct3d11, direct3d12, metal, vulkan and opengl.',
        value: true,
        default: GraphicsApi_1.GraphicsApi.Default
    },
    {
        full: 'ffmpeg',
        description: 'Location of ffmpeg executable',
        value: true,
        default: ''
    },
    {
        full: 'shaderversion',
        description: 'Set target shader version manually.',
        value: true,
        default: null
    },
    {
        full: 'parallelAssetConversion',
        description: 'Experimental - Spawn multiple processes during asset and shader conversion. Possible values:\n  0: disabled (default value)\n -1: choose number of processes automatically\n  N: specify number of processes manually',
        value: true,
        default: 0
    }
];
let parsedOptions = new Options_1.Options();
function printHelp() {
    console.log('khamake options:\n');
    for (let option of options) {
        if (option.hidden)
            continue;
        if (option.short)
            console.log('-' + option.short + ' ' + '--' + option.full);
        else
            console.log('--' + option.full);
        console.log(option.description);
        console.log();
    }
}
function isTarget(target) {
    if (target.trim().length < 1)
        return false;
    return true;
}
for (let option of options) {
    if (option.value) {
        parsedOptions[option.full] = option.default;
    }
    else {
        parsedOptions[option.full] = false;
    }
}
let args = process.argv;
for (let i = 2; i < args.length; ++i) {
    let arg = args[i];
    if (arg[0] === '-') {
        if (arg[1] === '-') {
            if (arg.substr(2) === 'help') {
                printHelp();
                process.exit(0);
            }
            for (let option of options) {
                if (arg.substr(2) === option.full) {
                    if (option.value) {
                        ++i;
                        parsedOptions[option.full] = args[i];
                    }
                    else {
                        parsedOptions[option.full] = true;
                    }
                }
            }
        }
        else {
            if (arg[1] === 'h') {
                printHelp();
                process.exit(0);
            }
            for (let option of options) {
                if (option.short && arg[1] === option.short) {
                    if (option.value) {
                        ++i;
                        parsedOptions[option.full] = args[i];
                    }
                    else {
                        parsedOptions[option.full] = true;
                    }
                }
            }
        }
    }
    else {
        if (isTarget(arg))
            parsedOptions.target = arg.toLowerCase();
    }
}
if (parsedOptions.run) {
    parsedOptions.compile = true;
}
async function runKhamake() {
    try {
        let logInfo = function (text, newline) {
            if (newline) {
                console.log(text);
            }
            else {
                process.stdout.write(text);
            }
        };
        let logError = function (text, newline) {
            if (newline) {
                console.error(text);
            }
            else {
                process.stderr.write(text);
            }
        };
        await require('./main.js').run(parsedOptions, { info: logInfo, error: logError }, (name) => { });
    }
    catch (error) {
        console.log(error);
        process.exit(1);
    }
}
runKhamake();
//# sourceMappingURL=khamake.js.map