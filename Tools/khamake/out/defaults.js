"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const GraphicsApi_1 = require("./GraphicsApi");
const Platform_1 = require("./Platform");
function graphicsApi(platform) {
    switch (platform) {
        case Platform_1.Platform.Empty:
        case Platform_1.Platform.Node:
        case Platform_1.Platform.Android:
        case Platform_1.Platform.HTML5:
        case Platform_1.Platform.DebugHTML5:
        case Platform_1.Platform.HTML5Worker:
        case Platform_1.Platform.Pi:
        case Platform_1.Platform.Linux:
            return GraphicsApi_1.GraphicsApi.OpenGL;
        case Platform_1.Platform.tvOS:
        case Platform_1.Platform.iOS:
        case Platform_1.Platform.OSX:
            return GraphicsApi_1.GraphicsApi.Metal;
        case Platform_1.Platform.Windows:
        case Platform_1.Platform.WindowsApp:
            return GraphicsApi_1.GraphicsApi.Direct3D11;
        case Platform_1.Platform.Krom:
            if (process.platform === 'win32') {
                return GraphicsApi_1.GraphicsApi.Direct3D11;
            }
            else if (process.platform === 'darwin') {
                return GraphicsApi_1.GraphicsApi.Metal;
            }
            else {
                return GraphicsApi_1.GraphicsApi.OpenGL;
            }
        default:
            return platform;
    }
}
exports.graphicsApi = graphicsApi;
//# sourceMappingURL=defaults.js.map