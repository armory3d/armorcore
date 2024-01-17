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

class Lz4 {

	static hashTable: Int32Array = null;

	static encodeBound = (size: i32): i32 => {
		return size > 0x7e000000 ? 0 : size + (size / 255 | 0) + 16;
	}

	static encode = (b: ArrayBuffer): ArrayBuffer => {
		let iBuf = new Uint8Array(b);
		let iLen = iBuf.length;
		if (iLen >= 0x7e000000) {
			Krom.log("LZ4 range error");
			return null;
		}

		// "The last match must start at least 12 bytes before end of block"
		let lastMatchPos = iLen - 12;

		// "The last 5 bytes are always literals"
		let lastLiteralPos = iLen - 5;

		if (Lz4.hashTable == null) Lz4.hashTable = new Int32Array(65536);
		for (let i = 0; i < Lz4.hashTable.length; ++i) {
			Lz4.hashTable[i] = -65536;
		}

		let oLen = Lz4.encodeBound(iLen);
		let oBuf = new Uint8Array(oLen);
		let iPos = 0;
		let oPos = 0;
		let anchorPos = 0;

		// Sequence-finding loop
		while (true) {
			let refPos = 0;
			let mOffset = 0;
			let sequence = iBuf[iPos] << 8 | iBuf[iPos + 1] << 16 | iBuf[iPos + 2] << 24;

			// Match-finding loop
			while (iPos <= lastMatchPos) {
				sequence = sequence >>> 8 | iBuf[iPos + 3] << 24;
				let hash = (sequence * 0x9e37 & 0xffff) + (sequence * 0x79b1 >>> 16) & 0xffff;
				refPos = Lz4.hashTable[hash];
				Lz4.hashTable[hash] = iPos;
				mOffset = iPos - refPos;
				if (mOffset < 65536 &&
					iBuf[refPos + 0] == ((sequence       ) & 0xff) &&
					iBuf[refPos + 1] == ((sequence >>>  8) & 0xff) &&
					iBuf[refPos + 2] == ((sequence >>> 16) & 0xff) &&
					iBuf[refPos + 3] == ((sequence >>> 24) & 0xff)
				) {
					break;
				}
				iPos += 1;
			}

			// No match found
			if (iPos > lastMatchPos) break;

			// Match found
			let lLen = iPos - anchorPos;
			let mLen = iPos;
			iPos += 4; refPos += 4;
			while (iPos < lastLiteralPos && iBuf[iPos] == iBuf[refPos]) {
				iPos += 1; refPos += 1;
			}
			mLen = iPos - mLen;
			let token = mLen < 19 ? mLen - 4 : 15;

			// Write token, length of literals if needed
			if (lLen >= 15) {
				oBuf[oPos++] = 0xf0 | token;
				let l = lLen - 15;
				while (l >= 255) {
					oBuf[oPos++] = 255;
					l -= 255;
				}
				oBuf[oPos++] = l;
			}
			else {
				oBuf[oPos++] = (lLen << 4) | token;
			}

			// Write literals
			while (lLen-- > 0) {
				oBuf[oPos++] = iBuf[anchorPos++];
			}

			if (mLen == 0) break;

			// Write offset of match
			oBuf[oPos + 0] = mOffset;
			oBuf[oPos + 1] = mOffset >>> 8;
			oPos += 2;

			// Write length of match if needed
			if (mLen >= 19) {
				let l = mLen - 19;
				while (l >= 255) {
					oBuf[oPos++] = 255;
					l -= 255;
				}
				oBuf[oPos++] = l;
			}

			anchorPos = iPos;
		}

		// Last sequence is literals only
		let lLen = iLen - anchorPos;
		if (lLen >= 15) {
			oBuf[oPos++] = 0xf0;
			let l = lLen - 15;
			while (l >= 255) {
				oBuf[oPos++] = 255;
				l -= 255;
			}
			oBuf[oPos++] = l;
		}
		else {
			oBuf[oPos++] = lLen << 4;
		}
		while (lLen-- > 0) {
			oBuf[oPos++] = iBuf[anchorPos++];
		}

		return oBuf.buffer.slice(0, oPos);
	}

	static decode = (b: ArrayBuffer, oLen: i32): ArrayBuffer => {
		let iBuf = new Uint8Array(b);
		let iLen = iBuf.length;
		let oBuf = new Uint8Array(oLen);
		let iPos = 0;
		let oPos = 0;

		while (iPos < iLen) {
			let token = iBuf[iPos++];

			// Literals
			let clen = token >>> 4;

			// Length of literals
			if (clen != 0) {
				if (clen == 15) {
					let l = 0;
					while (true) {
						l = iBuf[iPos++];
						if (l != 255) break;
						clen += 255;
					}
					clen += l;
				}

				// Copy literals
				let end = iPos + clen;
				while (iPos < end) {
					oBuf[oPos++] = iBuf[iPos++];
				}
				if (iPos == iLen) break;
			}

			// Match
			let mOffset = iBuf[iPos + 0] | (iBuf[iPos + 1] << 8);
			if (mOffset == 0 || mOffset > oPos) return null;
			iPos += 2;

			// Length of match
			clen = (token & 0x0f) + 4;
			if (clen == 19) {
				let l = 0;
				while (true) {
					l = iBuf[iPos++];
					if (l != 255) break;
					clen += 255;
				}
				clen += l;
			}

			// Copy match
			let mPos = oPos - mOffset;
			let end = oPos + clen;
			while (oPos < end) {
				oBuf[oPos++] = oBuf[mPos++];
			}
		}

		return oBuf.buffer;
	}
}
