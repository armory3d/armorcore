#ifdef KORE_MACOS

#import <Foundation/Foundation.h>

#include <CommonCrypto/CommonDigest.h>

#include <string>

std::string sha1(const char* data, int length) {
	CC_SHA1_CTX sha;
	CC_SHA1_Init(&sha);
	CC_SHA1_Update(&sha, data, length);
	char result[32];
	CC_SHA1_Final((unsigned char*)result, &sha);
	NSData* nsdata = [NSData dataWithBytes:result length:20];
	return [[nsdata base64Encoding] cStringUsingEncoding:NSUTF8StringEncoding];
}

#endif
