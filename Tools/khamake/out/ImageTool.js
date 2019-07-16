"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const child_process = require("child_process");
const fs = require("fs-extra");
const path = require("path");
const log = require("./log");
const exec_1 = require("./exec");
function getWidthAndHeight(kha, from, to, options, format, prealpha) {
    return new Promise((resolve, reject) => {
        const exe = 'kraffiti' + exec_1.sys();
        let params = ['from=' + from, 'to=' + to, 'format=' + format, 'donothing'];
        if (options.scale !== undefined && options.scale !== 1) {
            params.push('scale=' + options.scale);
        }
        let process = child_process.spawn(path.join(kha, 'Kinc', 'Tools', 'kraffiti', exe), params);
        let output = '';
        process.stdout.on('data', (data) => {
            output += data.toString();
        });
        process.stderr.on('data', (data) => {
        });
        process.on('close', (code) => {
            if (code !== 0) {
                log.error('kraffiti process exited with code ' + code + ' when trying to get size of ' + path.parse(from).name);
                resolve({ w: 0, h: 0 });
                return;
            }
            const lines = output.split('\n');
            for (let line of lines) {
                if (line.startsWith('#')) {
                    let numbers = line.substring(1).split('x');
                    resolve({ w: parseInt(numbers[0]), h: parseInt(numbers[1]) });
                    return;
                }
            }
            resolve({ w: 0, h: 0 });
        });
    });
}
function convertImage(from, temp, to, kha, exe, params, options, cache) {
    return new Promise((resolve, reject) => {
        let process = child_process.spawn(path.join(kha, 'Kinc', 'Tools', 'kraffiti', exe), params);
        let output = '';
        process.stdout.on('data', (data) => {
            output += data.toString();
        });
        process.stderr.on('data', (data) => {
        });
        process.on('close', (code) => {
            if (code !== 0) {
                log.error('kraffiti process exited with code ' + code + ' when trying to convert ' + path.parse(from).name);
                resolve();
                return;
            }
            fs.renameSync(temp, to);
            const lines = output.split('\n');
            for (let line of lines) {
                if (line.startsWith('#')) {
                    let numbers = line.substring(1).split('x');
                    cache[to] = {};
                    cache[to].original_width = options.original_width = parseInt(numbers[0]);
                    cache[to].original_height = options.original_height = parseInt(numbers[1]);
                    resolve();
                    return;
                }
            }
            resolve();
        });
    });
}
async function exportImage(kha, from, to, options, format, prealpha, poweroftwo, cache) {
    if (format === undefined) {
        if (from.toString().endsWith('.png'))
            format = 'png';
        else if (from.toString().endsWith('.hdr'))
            format = 'hdr';
        else
            format = 'jpg';
    }
    if (format === 'jpg' && (options.scale === undefined || options.scale === 1) && options.background === undefined) {
        to = to + '.jpg';
    }
    else if (format === 'pvr') {
        to = to + '.pvr';
    }
    else if (format === 'ASTC') {
        to = to + '.astc.k';
    }
    else if (format === 'DXT5') {
        to = to + '.dxt5.k';
    }
    else if (format === 'hdr') {
        to = to + '.hdr';
    }
    else if (format === 'lz4') {
        to += '.k';
    }
    else {
        format = 'png';
        if (prealpha)
            to = to + '.kng';
        else
            to = to + '.png';
    }
    let temp = to + '.temp';
    let outputformat = format;
    if (format === 'png' && prealpha) {
        outputformat = 'kng';
    }
    if (format === 'lz4') {
        outputformat = 'k';
    }
    if (format === 'ASTC') {
        outputformat = 'astc.k';
    }
    if (format === 'DXT5') {
        outputformat = 'dxt5.k';
    }
    if (fs.existsSync(to) && fs.statSync(to).mtime.getTime() > fs.statSync(from.toString()).mtime.getTime()) {
        if (cache[to] !== undefined) {
            const cachedOptions = cache[to];
            options.original_width = cachedOptions.original_width;
            options.original_height = cachedOptions.original_height;
            return outputformat;
        }
        let wh = await getWidthAndHeight(kha, from, to, options, format, prealpha);
        cache[to] = {};
        cache[to].original_width = options.original_width = wh.w;
        cache[to].original_height = options.original_height = wh.h;
        return outputformat;
    }
    fs.ensureDirSync(path.dirname(to));
    if (format === 'jpg' || format === 'hdr') {
        fs.copySync(from, temp, { overwrite: true });
        fs.renameSync(temp, to);
        let wh = await getWidthAndHeight(kha, from, to, options, format, prealpha);
        options.original_width = wh.w;
        options.original_height = wh.h;
        return outputformat;
    }
    const exe = 'kraffiti' + exec_1.sys();
    let params = ['from=' + from, 'to=' + temp, 'format=' + format];
    if (!poweroftwo) {
        params.push('filter=nearest');
    }
    if (prealpha)
        params.push('prealpha');
    if (options.scale !== undefined && options.scale !== 1) {
        params.push('scale=' + options.scale);
    }
    if (options.background !== undefined) {
        params.push('transparent=' + ((options.background.red << 24) | (options.background.green << 16) | (options.background.blue << 8) | 0xff).toString(16));
    }
    if (poweroftwo) {
        params.push('poweroftwo');
    }
    await convertImage(from, temp, to, kha, exe, params, options, cache);
    return outputformat;
}
exports.exportImage = exportImage;
//# sourceMappingURL=ImageTool.js.map