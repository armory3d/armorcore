#ifdef KORE_WINDOWS

#include <stdio.h>
#include <exception>
#include <ios>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <Kore/Log.h>

#include <Windows.h>
#include <bcrypt.h>

namespace {
	inline bool NtSuccess(NTSTATUS status)
	{
		return (status >= 0);
	}

	class CryptException :
		public std::runtime_error
	{
	public:

		// Constructs the exception with an error message and an error code.
		explicit CryptException(const std::string & message, NTSTATUS errorCode)
			: std::runtime_error(FormatErrorMessage(message, errorCode)),
			m_errorCode(errorCode)
		{}


		// Returns the error code.
		NTSTATUS ErrorCode() const
		{
			return m_errorCode;
		}


		//
		// IMPLEMENTATION
		//
	private:
		// Error code from Cryptography API
		NTSTATUS m_errorCode;

		// Helper method to format an error message including the error code.
		static std::string FormatErrorMessage(const std::string & message, NTSTATUS errorCode)
		{
			std::ostringstream os;
			os << message << " (NTSTATUS=0x" << std::hex << errorCode << ")";
			return os.str();
		}
	};

	class NonCopyable
	{
	protected:
		NonCopyable() {}
		~NonCopyable() {}
	private:
		NonCopyable(const NonCopyable&);
		const NonCopyable& operator=(const NonCopyable&);
	};

	class CryptAlgorithmProvider : NonCopyable
	{
	public:

		// Creates a crypt algorithm provider object.
		// This can be used to create one ore more hash objects to hash some data.
		CryptAlgorithmProvider()
		{
			NTSTATUS result = ::BCryptOpenAlgorithmProvider(
				&m_alg,                     // algorithm handle
				BCRYPT_SHA1_ALGORITHM,      // hashing algorithm ID
				nullptr,                    // use default provider
				0                           // default flags
			);
			if (!NtSuccess(result))
				throw CryptException("Can't load default cryptographic algorithm provider.", result);
		}


		// Releases resources
		~CryptAlgorithmProvider()
		{
			::BCryptCloseAlgorithmProvider(m_alg, 0);
		}


		// Gets raw handle
		BCRYPT_ALG_HANDLE Handle() const
		{
			return m_alg;
		}


		// Gets the value of a DWORD named property
		DWORD GetDWordProperty(const std::wstring & propertyName) const
		{
			DWORD propertyValue;
			DWORD resultSize;

			//
			// Get the value of the input named property
			//

			NTSTATUS result = ::BCryptGetProperty(
				Handle(),
				propertyName.c_str(),
				reinterpret_cast<BYTE *>(&propertyValue),
				sizeof(propertyValue),
				&resultSize,
				0);
			if (!NtSuccess(result))
				throw CryptException("Can't get crypt property value.", result);

			return propertyValue;
		}


		//
		// IMPLEMENTATION
		//
	private:
		// Handle to crypt algorithm provider
		BCRYPT_ALG_HANDLE m_alg;
	};



	//-----------------------------------------------------------------------------
	// Crypt Hash object, used to hash data.
	//-----------------------------------------------------------------------------
	class CryptHashObject : NonCopyable
	{
	public:

		// Creates a crypt hash object.
		explicit CryptHashObject(const CryptAlgorithmProvider & provider)
			: m_hashObj(provider.GetDWordProperty(BCRYPT_OBJECT_LENGTH))
		{
			// Create the hash object
			NTSTATUS result = ::BCryptCreateHash(
				provider.Handle(),  // handle to parent
				&m_hash,            // hash object handle
				m_hashObj.data(),   // hash object buffer pointer
				m_hashObj.size(),   // hash object buffer length
				nullptr,            // no secret
				0,                  // no secret
				0                   // no flags
			);
			if (!NtSuccess(result))
				throw CryptException("Can't create crypt hash object.", result);
		}


		// Releases resources
		~CryptHashObject()
		{
			::BCryptDestroyHash(m_hash);
		}


		// Hashes the data in the input buffer.
		// Can be called one or more times.
		// When finished with input data, call FinishHash().
		// This method can't be called after FinisHash() is called.
		void HashData(const void * data, unsigned long length) const
		{
			// Hash this chunk of data
			NTSTATUS result = ::BCryptHashData(
				m_hash, // hash object handle
				static_cast<UCHAR *>(const_cast<void *>(data)),    // safely remove const from buffer pointer
				length, // input buffer length in bytes
				0       // no flags
			);
			if (!NtSuccess(result))
				throw CryptException("Can't hash data.", result);
		}


		// Finalizes hash calculation.
		// After this method is called, hash value can be got using HashValue() method.
		// After this method is called, the HashData() method can't be called anymore.
		void FinishHash()
		{
			//
			// Retrieve the hash of the accumulated data
			//

			BYTE hashValue[20]; // SHA-1: 20 bytes = 160 bits

			NTSTATUS result = ::BCryptFinishHash(
				m_hash,             // handle to hash object
				hashValue,          // output buffer for hash value
				sizeof(hashValue),  // size of this buffer
				0                   // no flags
			);
			if (!NtSuccess(result))
				throw CryptException("Can't finalize hashing.", result);


			//
			// Get the hash digest string from hash value buffer.
			//

			// Each byte --> 2 hex chars
			/*m_hashDigest.resize(sizeof(hashValue) * 2);

			// Upper-case hex digits
			static const wchar_t hexDigits[] = L"0123456789ABCDEF";

			// Index to current character in destination string
			size_t currChar = 0;

			// For each byte in the hash value buffer
			for (size_t i = 0; i < sizeof(hashValue); ++i)
			{
				// high nibble
				m_hashDigest[currChar] = hexDigits[(hashValue[i] & 0xF0) >> 4];
				++currChar;

				// low nibble
				m_hashDigest[currChar] = hexDigits[(hashValue[i] & 0x0F)];
				++currChar;
			}*/

			DWORD size;
			BOOL success = CryptBinaryToStringA(hashValue, sizeof(hashValue), CRYPT_STRING_BASE64, m_hashDigest, &size);
			Kore::log(Kore::Info, "Crypt: %i", success); // This log fixes some optimization in VS2017 release mode
			
			/*m_hashDigest.resize(sizeof(hashValue));
			for (size_t i = 0; i < sizeof(hashValue); ++i) {
				m_hashDigest[i] = hashValue[i];
			}*/
		}


		// Gets the hash value (in hex, upper-case).
		// Call this method *after* FinishHash(), not before.
		// (If called before, an empty string is returned.)
		std::string HashDigest() const
		{
			return m_hashDigest;
		}


		//
		// IMPLEMENTATION
		//
	private:

		// Handle to hash object
		BCRYPT_HASH_HANDLE m_hash;

		// Buffer to store hash object
		std::vector<BYTE> m_hashObj;

		// Hash digest string (hex)
		char m_hashDigest[256];
	};

	std::string HashFileSHA1(const char* data, int length)
	{
		// Create the algorithm provider for SHA-1 hashing
		CryptAlgorithmProvider sha1;

		// Create the hash object for the particular hashing
		CryptHashObject hasher(sha1);

		hasher.HashData(data, length);
		
		hasher.FinishHash();

		return hasher.HashDigest();
	}
}

std::string sha1(const char* data, int length) {
	return HashFileSHA1(data, length);
}
#endif
