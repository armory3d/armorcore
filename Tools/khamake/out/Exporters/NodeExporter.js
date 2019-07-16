"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const Html5Exporter_1 = require("./Html5Exporter");
class NodeExporter extends Html5Exporter_1.Html5Exporter {
    constructor(options) {
        super(options);
    }
    backend() {
        return 'Node';
    }
}
exports.NodeExporter = NodeExporter;
//# sourceMappingURL=NodeExporter.js.map