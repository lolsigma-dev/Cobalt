#pragma once
#include "Crypt.hpp"
#include <cryptopp/base64.h>
#include <cryptopp/filters.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#include <cryptopp/md5.h>
#include <cryptopp/filters.h>

static int crypt_base64encode(lua_State* L) {
	size_t len;
	const char* data = luaL_checklstring(L, 1, &len);
	std::string encoded;
	CryptoPP::StringSource ss(reinterpret_cast<const uint8_t*>(data), len, true,
		new CryptoPP::Base64Encoder(new CryptoPP::StringSink(encoded), false));
	lua_pushlstring(L, encoded.data(), encoded.size());
	return 1;
}

static int crypt_base64decode(lua_State* L) {
	size_t len;
	const char* data = luaL_checklstring(L, 1, &len);
	std::string decoded;
	try {
		CryptoPP::StringSource ss(reinterpret_cast<const uint8_t*>(data), len, true,
			new CryptoPP::Base64Decoder(new CryptoPP::StringSink(decoded)));
		lua_pushlstring(L, decoded.data(), decoded.size());
	} catch (...) {
		lua_pushnil(L);
	}
	return 1;
}

static int crypt_encrypt(lua_State* L) {
	size_t dataLen, keyLen, ivLen;
	const char* dataStr = luaL_checklstring(L, 1, &dataLen);
	const char* keyStr = luaL_checklstring(L, 2, &keyLen);
	const char* ivStr = luaL_optlstring(L, 3, "", &ivLen);
	int modeArg = (int)luaL_optinteger(L, 4, 0);

	std::string key, iv;
	CryptoPP::StringSource(reinterpret_cast<const uint8_t*>(keyStr), keyLen, true,
		new CryptoPP::Base64Decoder(new CryptoPP::StringSink(key)));
	if (ivLen > 0) {
		CryptoPP::StringSource(reinterpret_cast<const uint8_t*>(ivStr), ivLen, true,
			new CryptoPP::Base64Decoder(new CryptoPP::StringSink(iv)));
	} else {
		iv.resize(16, 0);
	}

	try {
		std::string cipher;
		if (modeArg == 1) {
			CryptoPP::GCM<CryptoPP::AES>::Encryption e;
			e.SetKeyWithIV(reinterpret_cast<const uint8_t*>(key.data()), key.size(),
				reinterpret_cast<const uint8_t*>(iv.data()), iv.size());
			CryptoPP::StringSource ss(reinterpret_cast<const uint8_t*>(dataStr), dataLen, true,
				new CryptoPP::AuthenticatedEncryptionFilter(e, new CryptoPP::StringSink(cipher)));
		} else {
			CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption e;
			e.SetKeyWithIV(reinterpret_cast<const uint8_t*>(key.data()), key.size(),
				reinterpret_cast<const uint8_t*>(iv.data()), iv.size());
			CryptoPP::StringSource ss(reinterpret_cast<const uint8_t*>(dataStr), dataLen, true,
				new CryptoPP::StreamTransformationFilter(e, new CryptoPP::StringSink(cipher)));
		}

		std::string encoded;
		CryptoPP::StringSource(reinterpret_cast<const uint8_t*>(cipher.data()), cipher.size(), true,
			new CryptoPP::Base64Encoder(new CryptoPP::StringSink(encoded), false));

		std::string ivEncoded;
		CryptoPP::StringSource(reinterpret_cast<const uint8_t*>(iv.data()), iv.size(), true,
			new CryptoPP::Base64Encoder(new CryptoPP::StringSink(ivEncoded), false));

		lua_pushlstring(L, encoded.data(), encoded.size());
		lua_pushlstring(L, ivEncoded.data(), ivEncoded.size());
		return 2;
	} catch (...) {
		lua_pushnil(L);
		return 1;
	}
}

static int crypt_decrypt(lua_State* L) {
	size_t dataLen, keyLen, ivLen;
	const char* dataStr = luaL_checklstring(L, 1, &dataLen);
	const char* keyStr = luaL_checklstring(L, 2, &keyLen);
	const char* ivStr = luaL_checklstring(L, 3, &ivLen);
	int modeArg = (int)luaL_optinteger(L, 4, 0);

	std::string cipher, key, iv;
	CryptoPP::StringSource(reinterpret_cast<const uint8_t*>(dataStr), dataLen, true,
		new CryptoPP::Base64Decoder(new CryptoPP::StringSink(cipher)));
	CryptoPP::StringSource(reinterpret_cast<const uint8_t*>(keyStr), keyLen, true,
		new CryptoPP::Base64Decoder(new CryptoPP::StringSink(key)));
	CryptoPP::StringSource(reinterpret_cast<const uint8_t*>(ivStr), ivLen, true,
		new CryptoPP::Base64Decoder(new CryptoPP::StringSink(iv)));

	try {
		std::string plain;
		if (modeArg == 1) {
			CryptoPP::GCM<CryptoPP::AES>::Decryption e;
			e.SetKeyWithIV(reinterpret_cast<const uint8_t*>(key.data()), key.size(),
				reinterpret_cast<const uint8_t*>(iv.data()), iv.size());
			CryptoPP::StringSource ss(reinterpret_cast<const uint8_t*>(cipher.data()), cipher.size(), true,
				new CryptoPP::AuthenticatedDecryptionFilter(e, new CryptoPP::StringSink(plain)));
		} else {
			CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption e;
			e.SetKeyWithIV(reinterpret_cast<const uint8_t*>(key.data()), key.size(),
				reinterpret_cast<const uint8_t*>(iv.data()), iv.size());
			CryptoPP::StringSource ss(reinterpret_cast<const uint8_t*>(cipher.data()), cipher.size(), true,
				new CryptoPP::StreamTransformationFilter(e, new CryptoPP::StringSink(plain)));
		}
		lua_pushlstring(L, plain.data(), plain.size());
		return 1;
	} catch (...) {
		lua_pushnil(L);
		return 1;
	}
}

static int crypt_generatebytes(lua_State* L) {
	int size = (int)luaL_checkinteger(L, 1);
	if (size <= 0) size = 1;
	std::string bytes(size, '\0');
	CryptoPP::OS_GenerateRandomBlock(false, reinterpret_cast<uint8_t*>(bytes.data()), size);
	std::string encoded;
	CryptoPP::StringSource(reinterpret_cast<const uint8_t*>(bytes.data()), size, true,
		new CryptoPP::Base64Encoder(new CryptoPP::StringSink(encoded), false));
	lua_pushlstring(L, encoded.data(), encoded.size());
	return 1;
}

static int crypt_generatekey(lua_State* L) {
	std::string key(32, '\0');
	CryptoPP::OS_GenerateRandomBlock(false, reinterpret_cast<uint8_t*>(key.data()), 32);
	std::string encoded;
	CryptoPP::StringSource(reinterpret_cast<const uint8_t*>(key.data()), 32, true,
		new CryptoPP::Base64Encoder(new CryptoPP::StringSink(encoded), false));
	lua_pushlstring(L, encoded.data(), encoded.size());
	return 1;
}

static int crypt_hash(lua_State* L) {
	size_t len;
	const char* data = luaL_checklstring(L, 1, &len);
	const char* algorithm = luaL_checkstring(L, 2);

	std::string digest;
	try {
		if (_stricmp(algorithm, "md5") == 0) {
			CryptoPP::Weak1::MD5 hash;
			CryptoPP::StringSource(reinterpret_cast<const uint8_t*>(data), len, true,
				new CryptoPP::HashFilter(hash, new CryptoPP::HexEncoder(new CryptoPP::StringSink(digest), false)));
		} else if (_stricmp(algorithm, "sha1") == 0) {
			CryptoPP::SHA1 hash;
			CryptoPP::StringSource(reinterpret_cast<const uint8_t*>(data), len, true,
				new CryptoPP::HashFilter(hash, new CryptoPP::HexEncoder(new CryptoPP::StringSink(digest), false)));
		} else if (_stricmp(algorithm, "sha256") == 0) {
			CryptoPP::SHA256 hash;
			CryptoPP::StringSource(reinterpret_cast<const uint8_t*>(data), len, true,
				new CryptoPP::HashFilter(hash, new CryptoPP::HexEncoder(new CryptoPP::StringSink(digest), false)));
		} else if (_stricmp(algorithm, "sha384") == 0) {
			CryptoPP::SHA384 hash;
			CryptoPP::StringSource(reinterpret_cast<const uint8_t*>(data), len, true,
				new CryptoPP::HashFilter(hash, new CryptoPP::HexEncoder(new CryptoPP::StringSink(digest), false)));
		} else if (_stricmp(algorithm, "sha512") == 0) {
			CryptoPP::SHA512 hash;
			CryptoPP::StringSource(reinterpret_cast<const uint8_t*>(data), len, true,
				new CryptoPP::HashFilter(hash, new CryptoPP::HexEncoder(new CryptoPP::StringSink(digest), false)));
		} else if (_stricmp(algorithm, "sha3-224") == 0) {
			CryptoPP::SHA3_224 hash;
			CryptoPP::StringSource(reinterpret_cast<const uint8_t*>(data), len, true,
				new CryptoPP::HashFilter(hash, new CryptoPP::HexEncoder(new CryptoPP::StringSink(digest), false)));
		} else if (_stricmp(algorithm, "sha3-256") == 0) {
			CryptoPP::SHA3_256 hash;
			CryptoPP::StringSource(reinterpret_cast<const uint8_t*>(data), len, true,
				new CryptoPP::HashFilter(hash, new CryptoPP::HexEncoder(new CryptoPP::StringSink(digest), false)));
		} else if (_stricmp(algorithm, "sha3-512") == 0) {
			CryptoPP::SHA3_512 hash;
			CryptoPP::StringSource(reinterpret_cast<const uint8_t*>(data), len, true,
				new CryptoPP::HashFilter(hash, new CryptoPP::HexEncoder(new CryptoPP::StringSink(digest), false)));
		} else {
			lua_pushnil(L);
			lua_pushstring(L, "Unsupported algorithm");
			return 2;
		}
		lua_pushlstring(L, digest.data(), digest.size());
		return 1;
	} catch (...) {
		lua_pushnil(L);
		return 1;
	}
}

static int crypt_create(lua_State* L) {
	lua_newtable(L);
	lua_pushcfunction(L, crypt_base64encode);
	lua_setfield(L, -2, "base64encode");
	lua_pushcfunction(L, crypt_base64decode);
	lua_setfield(L, -2, "base64decode");
	lua_pushcfunction(L, crypt_encrypt);
	lua_setfield(L, -2, "encrypt");
	lua_pushcfunction(L, crypt_decrypt);
	lua_setfield(L, -2, "decrypt");
	lua_pushcfunction(L, crypt_generatebytes);
	lua_setfield(L, -2, "generatebytes");
	lua_pushcfunction(L, crypt_generatekey);
	lua_setfield(L, -2, "generatekey");
	lua_pushcfunction(L, crypt_hash);
	lua_setfield(L, -2, "hash");
	return 1;
}

void CCrypt::InitLib(lua_State* L) {
	declare__Global(L, "crypt", crypt_create);
}
