#include "../Platform/stdafx.h"
#if defined(_WIN32)
#include <xhash>
#else
#include <openssl/md5.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#endif // _WIN32
#include "Hasher.h"
#include <openssl/evp.h>

Hasher::Hasher(std::wstring &salt)
{
	this->salt = salt;
}

std::wstring Hasher::getHash(std::wstring &name)
{
#if defined(_WIN32)
	// 4J Stu - Removed try/catch
	//try {
		std::wstring s = std::wstring( salt ).append( name );
		//MessageDigest m;
		//m = MessageDigest.getInstance("MD5");
		//m.update(s.getBytes(), 0, s.length());
		//return new BigInteger(1, m.digest()).toString(16);

		// TODO 4J Stu - Will this hash us with the same distribution as the MD5?
		return _toString( hash_value( s ) );
	//}
	//catch (NoSuchAlgorithmException e)
	//{
	//	throw new RuntimeException(e);
	//}
#else
	// adapted from a SSL example
	std::wstring combined = salt + name;
	std::string combined_str(combined.begin(), combined.end());
	unsigned char result[EVP_MAX_MD_SIZE];
	EVP_MD_CTX* md5_ctx = EVP_MD_CTX_new();
	EVP_DigestInit_ex(md5_ctx, EVP_md5(), NULL);
	EVP_DigestUpdate(md5_ctx, combined_str.c_str(), combined_str.size());
	unsigned int result_len;
	EVP_DigestFinal_ex(md5_ctx, result, &result_len);
	EVP_MD_CTX_free(md5_ctx);
	std::stringstream ss;
	for (unsigned int i = 0; i < result_len; i++)
	{
		ss << std::setw(2) << std::setfill('0') << std::hex << (int)result[i];
	}
	std::string hash_str = ss.str();
	return std::wstring(hash_str.begin(), hash_str.end());
#endif
}
