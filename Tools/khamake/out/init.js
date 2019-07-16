"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const fs = require("fs");
const path = require("path");
function run(name, from, projectfile) {
    if (!fs.existsSync(path.join(from, projectfile))) {
        fs.writeFileSync(path.join(from, projectfile), 'let project = new Project(\'New Project\');\n'
            + 'project.addAssets(\'Assets/**\');\n'
            + 'project.addShaders(\'Shaders/**\');\n'
            + 'project.addSources(\'Sources\');\n'
            + 'resolve(project);\n', { encoding: 'utf8' });
    }
    if (!fs.existsSync(path.join(from, 'Assets')))
        fs.mkdirSync(path.join(from, 'Assets'));
    if (!fs.existsSync(path.join(from, 'Shaders')))
        fs.mkdirSync(path.join(from, 'Shaders'));
    if (!fs.existsSync(path.join(from, 'Sources')))
        fs.mkdirSync(path.join(from, 'Sources'));
    let friendlyName = name;
    friendlyName = friendlyName.replace(/ /g, '_');
    friendlyName = friendlyName.replace(/-/g, '_');
    if (!fs.existsSync(path.join(from, 'Sources', 'Main.hx'))) {
        let mainsource = 'package;\n\n'
            + 'import kha.Assets;\n'
            + 'import kha.Color;\n'
            + 'import kha.Framebuffer;\n'
            + 'import kha.Scheduler;\n'
            + 'import kha.System;\n\n'
            + 'class Main {\n'
            + '\tstatic var logo = ["1 1 1 1 111", "11  111 111", "1 1 1 1 1 1"];\n\n'
            + '\tstatic function update(): Void {\n'
            + '\t}\n\n'
            + '\tstatic function render(frames: Array<Framebuffer>): Void {\n'
            + '\t\t// As we are using only 1 window, grab the first framebuffer\n'
            + '\t\tfinal fb = frames[0];\n'
            + '\t\t// Now get the `g2` graphics object so we can draw\n'
            + '\t\tfinal g2 = fb.g2;\n'
            + '\t\t// Start drawing, and clear the framebuffer to `petrol`\n'
            + '\t\tg2.begin(true, Color.fromBytes(0, 95, 106));\n'
            + '\t\t// Offset all following drawing operations from the top-left a bit\n'
            + '\t\tg2.pushTranslation(64, 64);\n'
            + '\t\t// Fill the following rects with red\n'
            + '\t\tg2.color = Color.Red;\n\n'
            + '\t\t// Loop over the logo (Array<String>) and draw a rect for each "1"\n'
            + '\t\tfor (rowIndex in 0...logo.length) {\n'
            + '\t\t  final row = logo[rowIndex];\n\n'
            + '\t\t  for (colIndex in 0...row.length) {\n'
            + '\t\t    switch row.charAt(colIndex) {\n'
            + '\t\t      case "1": g2.fillRect(colIndex * 16, rowIndex * 16, 16, 16);\n'
            + '\t\t      case _:\n'
            + '\t\t    }\n'
            + '\t\t  }\n'
            + '\t\t}\n\n'
            + '\t\t// Pop the pushed translation so it will not accumulate over multiple frames\n'
            + '\t\tg2.popTransformation();\n'
            + '\t\t// Finish the drawing operations\n'
            + '\t\tg2.end();\n'
            + '\t}\n\n'
            + '\tpublic static function main() {\n'
            + '\t\tSystem.start({title: "' + name + '", width: 1024, height: 768}, function (_) {\n'
            + '\t\t\t// Just loading everything is ok for small projects\n'
            + '\t\t\tAssets.loadEverything(function () {\n'
            + '\t\t\t\t// Avoid passing update/render directly,\n'
            + '\t\t\t\t// so replacing them via code injection works\n'
            + '\t\t\t\tScheduler.addTimeTask(function () { update(); }, 0, 1 / 60);\n'
            + '\t\t\t\tSystem.notifyOnFrames(function (frames) { render(frames); });\n'
            + '\t\t\t});\n'
            + '\t\t});\n'
            + '\t}\n'
            + '}\n';
        fs.writeFileSync(path.join(from, 'Sources', 'Main.hx'), mainsource, { encoding: 'utf8' });
    }
}
exports.run = run;
//# sourceMappingURL=init.js.map