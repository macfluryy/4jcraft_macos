#include "stdafx.h"
#if defined(_WIN32)
#include <xhash>
#else
#include <openssl/md5.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#endif // _WIN32
#include "Hasher.h"

Hasher::Hasher(wstring &salt)
{
	this->salt = salt;
}

wstring Hasher::getHash(wstring &name)
{
#if defined(_WIN32)
	// 4J Stu - Removed try/catch
	//try {
		wstring s = wstring( salt ).append( name );
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
	unsigned char result[MD5_DIGEST_LENGTH];
	MD5_CTX md5_ctx;
	MD5_Init(&md5_ctx);
	MD5_Update(&md5_ctx, combined_str.c_str(), combined_str.size());
	MD5_Final(result, &md5_ctx);
	std::stringstream ss;
	for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
	{
		ss << std::setw(2) << std::setfill('0') << std::hex << (int)result[i];
	}
	std::string hash_str = ss.str();
	return std::wstring(hash_str.begin(), hash_str.end());
#endif
}
