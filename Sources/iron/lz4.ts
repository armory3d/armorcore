// Based on https://github.com/gorhill/lz4-wasm
// BSD 2-Clause License
// Copyright (c) 2018, Raymond Hill
// All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

let _lz4_hash_table: Int32Array = null;

function lz4_encode_bound(size: i32): i32 {
	return size > 0x7e000000 ? 0 : size + (size / 255 | 0) + 16;
}

function lz4_encode(b: ArrayBuffer): ArrayBuffer {
	let ibuf: Uint8Array = new Uint8Array(b);
	let ilen: i32 = ibuf.length;
	if (ilen >= 0x7e000000) {
		krom_log("LZ4 range error");
		return null;
	}

	// "The last match must start at least 12 bytes before end of block"
	let last_match_pos: i32 = ilen - 12;

	// "The last 5 bytes are always literals"
	let last_literal_pos: i32 = ilen - 5;

	if (_lz4_hash_table == null) {
		_lz4_hash_table = new Int32Array(65536);
	}
	for (let i: i32 = 0; i < _lz4_hash_table.length; ++i) {
		_lz4_hash_table[i] = -65536;
	}

	let olen: i32 = lz4_encode_bound(ilen);
	let obuf: Uint8Array = new Uint8Array(olen);
	let ipos: i32 = 0;
	let opos: i32 = 0;
	let anchor_pos: i32 = 0;

	// Sequence-finding loop
	while (true) {
		let ref_pos: i32 = 0;
		let moffset: i32 = 0;
		let sequence: i32 = ibuf[ipos] << 8 | ibuf[ipos + 1] << 16 | ibuf[ipos + 2] << 24;

		// Match-finding loop
		while (ipos <= last_match_pos) {
			sequence = sequence >>> 8 | ibuf[ipos + 3] << 24;
			let hash: i32 = (sequence * 0x9e37 & 0xffff) + (sequence * 0x79b1 >>> 16) & 0xffff;
			ref_pos = _lz4_hash_table[hash];
			_lz4_hash_table[hash] = ipos;
			moffset = ipos - ref_pos;
			if (moffset < 65536 &&
				ibuf[ref_pos + 0] == ((sequence       ) & 0xff) &&
				ibuf[ref_pos + 1] == ((sequence >>>  8) & 0xff) &&
				ibuf[ref_pos + 2] == ((sequence >>> 16) & 0xff) &&
				ibuf[ref_pos + 3] == ((sequence >>> 24) & 0xff)
			) {
				break;
			}
			ipos += 1;
		}

		// No match found
		if (ipos > last_match_pos) {
			break;
		}

		// Match found
		let llen: i32 = ipos - anchor_pos;
		let mlen: i32 = ipos;
		ipos += 4; ref_pos += 4;
		while (ipos < last_literal_pos && ibuf[ipos] == ibuf[ref_pos]) {
			ipos += 1; ref_pos += 1;
		}
		mlen = ipos - mlen;
		let token: i32 = mlen < 19 ? mlen - 4 : 15;

		// Write token, length of literals if needed
		if (llen >= 15) {
			obuf[opos++] = 0xf0 | token;
			let l: i32 = llen - 15;
			while (l >= 255) {
				obuf[opos++] = 255;
				l -= 255;
			}
			obuf[opos++] = l;
		}
		else {
			obuf[opos++] = (llen << 4) | token;
		}

		// Write literals
		while (llen-- > 0) {
			obuf[opos++] = ibuf[anchor_pos++];
		}

		if (mlen == 0) {
			break;
		}

		// Write offset of match
		obuf[opos + 0] = moffset;
		obuf[opos + 1] = moffset >>> 8;
		opos += 2;

		// Write length of match if needed
		if (mlen >= 19) {
			let l: i32 = mlen - 19;
			while (l >= 255) {
				obuf[opos++] = 255;
				l -= 255;
			}
			obuf[opos++] = l;
		}

		anchor_pos = ipos;
	}

	// Last sequence is literals only
	let llen: i32 = ilen - anchor_pos;
	if (llen >= 15) {
		obuf[opos++] = 0xf0;
		let l: i32 = llen - 15;
		while (l >= 255) {
			obuf[opos++] = 255;
			l -= 255;
		}
		obuf[opos++] = l;
	}
	else {
		obuf[opos++] = llen << 4;
	}
	while (llen-- > 0) {
		obuf[opos++] = ibuf[anchor_pos++];
	}

	return obuf.buffer.slice(0, opos);
}

function lz4_decode(b: ArrayBuffer, olen: i32): ArrayBuffer {
	let ibuf: Uint8Array = new Uint8Array(b);
	let ilen: i32 = ibuf.length;
	let obuf: Uint8Array = new Uint8Array(olen);
	let ipos: i32 = 0;
	let opos: i32 = 0;

	while (ipos < ilen) {
		let token: i32 = ibuf[ipos++];

		// Literals
		let clen: i32 = token >>> 4;

		// Length of literals
		if (clen != 0) {
			if (clen == 15) {
				let l: i32 = 0;
				while (true) {
					l = ibuf[ipos++];
					if (l != 255) {
						break;
					}
					clen += 255;
				}
				clen += l;
			}

			// Copy literals
			let end: i32 = ipos + clen;
			while (ipos < end) {
				obuf[opos++] = ibuf[ipos++];
			}
			if (ipos == ilen) {
				break;
			}
		}

		// Match
		let moffset: i32 = ibuf[ipos + 0] | (ibuf[ipos + 1] << 8);
		if (moffset == 0 || moffset > opos) {
			return null;
		}
		ipos += 2;

		// Length of match
		clen = (token & 0x0f) + 4;
		if (clen == 19) {
			let l: i32 = 0;
			while (true) {
				l = ibuf[ipos++];
				if (l != 255) {
					break;
				}
				clen += 255;
			}
			clen += l;
		}

		// Copy match
		let mpos: i32 = opos - moffset;
		let end: i32 = opos + clen;
		while (opos < end) {
			obuf[opos++] = obuf[mpos++];
		}
	}

	return obuf.buffer;
}
