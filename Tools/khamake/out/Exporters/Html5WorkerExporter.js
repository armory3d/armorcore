"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const Html5Exporter_1 = require("./Html5Exporter");
class Html5WorkerExporter extends Html5Exporter_1.Html5Exporter {
    constructor(options) {
        super(options);
    }
    backend() {
        return 'HTML5-Worker';
    }
}
exports.Html5WorkerExporter = Html5WorkerExporter;
//# sourceMappingURL=Html5WorkerExporter.js.map