
// declare type c_ptr = { __kind: "c_ptr"; };
// declare type c_int = number;
// declare type c_float = number;
// declare type c_char = number;
// declare type c_uchar = number;
// declare function print(s: string): void;

"use strict";

(function () {

    const c_null = $SHBuiltin.c_null();
    const _ptr_write_char = $SHBuiltin.extern_c({declared: true}, function _sh_ptr_write_char(ptr: c_ptr, offset: c_int, v: c_char): void {});
    const _ptr_read_uchar = $SHBuiltin.extern_c({declared: true}, function _sh_ptr_read_uchar(ptr: c_ptr, offset: c_int): c_uchar { throw 0; });
    const _malloc = $SHBuiltin.extern_c({include: "stdlib.h"}, function malloc(size: c_size_t): c_ptr { throw 0; });
    const _free = $SHBuiltin.extern_c({include: "stdlib.h"}, function free(p: c_ptr): void {});

    function to_c_string(s: any): c_ptr {
        "use unsafe";
        let buf = _malloc(s.length + 1);
        let i = 0;
        for (let l = s.length; i < l; ++i) {
            let code: number = s.charCodeAt(i);
            _ptr_write_char(buf, i, code);
        }
        _ptr_write_char(buf, i, 0);
        return buf;
    }

    function to_js_string(buf: c_ptr): string {
        let res = "";
        for (let i = 0; ; ++i) {
            let ch = _ptr_read_uchar(buf, i);
            if (ch == 0) break;
            res += String.fromCharCode(ch);
        }
        return res;
    }

    let Krom = globalThis.Krom = {};

    let krom_init = $SHBuiltin.extern_c({}, function _krom_init(title: c_ptr, w: c_int, h: c_int): void {});
    Krom.init = function(title: string, w: number, h: number) {
        // "inline";
        krom_init(to_c_string(title), w, h);
    };

    let krom_set_callback = $SHBuiltin.extern_c({}, function _krom_set_callback(callback: c_ptr): void {});
    Krom.setCallback = function(callback: ()=>void) {
        // "use unsafe";
        // krom_set_callback(callback);
    }

})({});
