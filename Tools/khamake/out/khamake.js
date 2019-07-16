"use strict";
// Called from entry point, e.g. Kha/make.js
// This is where options are processed:
// e.g. '-t html5 --server'
Object.defineProperty(exports, "__esModule", { value: true });
const os = require("os");
const path = require("path");
const GraphicsApi_1 = require("./GraphicsApi");
const Architecture_1 = require("./Architecture");
const AudioApi_1 = require("./AudioApi");
const VrApi_1 = require("./VrApi");
const RayTraceApi_1 = require("./RayTraceApi");
const Options_1 = require("./Options");
const Platform_1 = require("./Platform");
const VisualStudioVersion_1 = require("./VisualStudioVersion");
let defaultTarget;
if (os.platform() === 'linux') {
    defaultTarget = Platform_1.Platform.Linux;
}
else if (os.platform() === 'win32') {
    defaultTarget = Platform_1.Platform.Windows;
}
else {
    defaultTarget = Platform_1.Platform.OSX;
}
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
        full: 'projectfile',
        value: true,
        description: 'Name of your project file, defaults to "khafile.js"',
        default: 'khafile.js'
    },
    {
        full: 'target',
        short: 't',
        value: true,
        description: 'Target platform',
        default: defaultTarget
    },
    {
        full: 'vr',
        value: true,
        description: 'Target VR device',
        default: VrApi_1.VrApi.None
    },
    {
        full: 'raytrace',
        value: true,
        description: 'Target raytracing api',
        default: RayTraceApi_1.RayTraceApi.None
    },
    {
        full: 'main',
        value: true,
        description: 'Entrypoint for the haxe code (-main argument), defaults to "Main".',
        default: 'Main'
    },
    {
        full: 'intermediate',
        description: 'Intermediate location for object files.',
        value: true,
        default: '',
        hidden: true
    },
    {
        full: 'graphics',
        short: 'g',
        description: 'Graphics api to use. Possible parameters are direct3d9, direct3d11, direct3d12, metal, vulkan and opengl.',
        value: true,
        default: GraphicsApi_1.GraphicsApi.Default
    },
    {
        full: 'arch',
        description: 'Target architecture to use. Possible parameters are arm7, arm8, x86, x86_64.',
        value: true,
        default: Architecture_1.Architecture.Default
    },
    {
        full: 'audio',
        short: 'a',
        description: 'Audio api to use. Possible parameters are directsound and wasapi.',
        value: true,
        default: AudioApi_1.AudioApi.Default
    },
    {
        full: 'visualstudio',
        short: 'v',
        description: 'Version of Visual Studio to use. Possible parameters are vs2010, vs2012, vs2013 and vs2015.',
        value: true,
        default: VisualStudioVersion_1.VisualStudioVersion.VS2019
    },
    {
        full: 'kha',
        short: 'k',
        description: 'Location of Kha directory',
        value: true,
        default: ''
    },
    {
        full: 'haxe',
        description: 'Location of Haxe directory',
        value: true,
        default: ''
    },
    {
        full: 'nohaxe',
        description: 'Do not compile Haxe sources',
        value: false,
    },
    {
        full: 'ffmpeg',
        description: 'Location of ffmpeg executable',
        value: true,
        default: ''
    },
    {
        full: 'ogg',
        description: 'Commandline for running the ogg encoder',
        value: true,
        default: ''
    },
    {
        full: 'mp3',
        description: 'Commandline for running the mp3 encoder',
        value: true,
        default: ''
    },
    {
        full: 'aac',
        description: 'Commandline for running the ffmpeg executable',
        value: true,
        default: ''
    },
    {
        full: 'krafix',
        description: 'Location of krafix shader compiler',
        value: true,
        default: ''
    },
    {
        full: 'noshaders',
        description: 'Do not compile shaders',
        value: false
    },
    {
        full: 'noproject',
        description: 'Only source files. Don\'t generate project files.',
        value: false,
    },
    {
        full: 'onlydata',
        description: 'Only assets/data. Don\'t generate project files.',
        value: false,
    },
    {
        full: 'embedflashassets',
        description: 'Embed assets in swf for flash target',
        value: false
    },
    {
        full: 'compile',
        description: 'Compile executable',
        value: false
    },
    {
        full: 'run',
        description: 'Run executable',
        value: false
    },
    {
        full: 'init',
        description: 'Init a Kha project inside the current directory',
        value: false
    },
    {
        full: 'name',
        description: 'Project name to use when initializing a project',
        value: true,
        default: 'Project'
    },
    {
        full: 'server',
        description: 'Run local http server for html5 target',
        value: false
    },
    {
        full: 'port',
        description: 'Running port for the server',
        value: true,
        default: 8080
    },
    {
        full: 'debug',
        description: 'Compile in debug mode.',
        value: false
    },
    {
        full: 'silent',
        description: 'Silent mode.',
        value: false
    },
    {
        full: 'watch',
        short: 'w',
        description: 'Watch files and recompile on change.',
        value: false
    },
    {
        full: 'glsl2',
        description: 'Use experimental SPIRV-Cross glsl mode.',
        value: false
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
    },
    {
        full: 'haxe3',
        description: 'Use the battle tested Haxe 3 compiler instead of the cutting edge not really released yet Haxe 4 compiler',
        value: false
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
if (parsedOptions.init) {
    console.log('Initializing Kha project.\n');
    require('./init').run(parsedOptions.name, parsedOptions.from, parsedOptions.projectfile);
    console.log('If you want to use the git version of Kha, execute "git init" and "git submodule add https://github.com/Kode/Kha.git".');
}
else if (parsedOptions.server) {
    console.log('Running server on ' + parsedOptions.port);
    let nstatic = require('node-static');
    let fileServer = new nstatic.Server(path.join(parsedOptions.from, 'build', 'html5'), { cache: 0 });
    let server = require('http').createServer(function (request, response) {
        request.addListener('end', function () {
            fileServer.serve(request, response);
        }).resume();
    });
    server.on('error', function (e) {
        if (e.code === 'EADDRINUSE') {
            console.log('Error: Port ' + parsedOptions.port + ' is already in use.');
            console.log('Please close the competing program (maybe another instance of khamake?)');
            console.log('or switch to a different port using the --port argument.');
        }
    });
    server.listen(parsedOptions.port);
}
else {
    runKhamake();
}
//# sourceMappingURL=khamake.js.map