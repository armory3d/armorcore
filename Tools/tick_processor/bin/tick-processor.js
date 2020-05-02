#!/usr/bin/env node

'use strict';

const childProcess = require('child_process');
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const scripts = [];

{
  for (const js of [
    'splaytree.js',
    'codemap.js',
    'csvparser.js',
    'consarray.js',
    'profile.js',
    'profile_view.js',
    'logreader.js',
    'arguments.js',
    'tickprocessor.js',
    'SourceMap.js',
    'tickprocessor-driver.js'
  ]) {
    const filename = path.join(__dirname, '..', 'lib', js);
    const code = fs.readFileSync(filename).toString();
    scripts.push(new vm.Script(code, { filename, displayErrors: true }));
  }
}

function print (...msg) {
  console.log(...msg);
}

function printErr (...msg) {
  console.error(...msg);
}

function read (filename) {
  return fs.readFileSync(filename).toString();
}

function quit (status) {
  process.exit(status);
}

function write (...msg) {
  process.stdout.write(...msg);
}

class os {
  static system (file, args) {
    return childProcess.execFileSync(file, args).toString();
  }
}

// Inject emulation of d8 environment to global context
Object.assign(global, {
  arguments: process.argv.slice(2),
  os,
  print,
  printErr,
  read,
  quit,
  write
});

// All but last script
for (const script of scripts.slice(0, -1)) {
  script.runInThisContext();
}

// Monkey patching before last script:

// On NodeJS we use fs.readFileSync rather than readline() from d8
TickProcessor.prototype.processLogFile = TickProcessor.prototype.processLogFileInTest;

// Different default settings based on Node platform
const getDefaultResultsOriginal = ArgumentsProcessor.prototype.getDefaultResults;
ArgumentsProcessor.prototype.getDefaultResults = () => {
  const results = getDefaultResultsOriginal();
  if (process.platform === 'darwin') {
    results.platform = 'mac';
    results.nm = path.join(__dirname, 'mac-nm');
  } else if (process.platform === 'win32') {
    results.platform = 'windows';
  }
  return results;
};

// Last script is an entry point
scripts.slice(-1)[0].runInThisContext();
