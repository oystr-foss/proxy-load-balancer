//
// Created by realngnx on 11/06/2021.
//
#include <openssl/sha.h>
#include <openssl/md5.h>
#include "./hasher.hpp"


long Digest::to_md5_hash(std::string key) {
    const char * hashed = reinterpret_cast<const char *>(md5(key));
    return calculate_hash(hashed);
}

long Digest::to_sha256_hash(std::string key) {
    const char * hashed = reinterpret_cast<const char *>(sha256(key));
    return calculate_hash(hashed);
}

long Digest::calculate_hash(const char *hashed) {
    long hash = 0;
    for (int i = 0; i < 4; i++) {
        hash <<= 8;
        hash |= ((int) hashed[i]) & 0xFF;
    }
    return hash;
}

const unsigned char * Digest::sha256(const std::string &key) {
    char const *c = key.c_str();
    return SHA256(reinterpret_cast<const unsigned char *>(c), key.length(), nullptr);
}

const unsigned char * Digest::md5(const std::string &key) {
    char const *c = key.c_str();
    return MD5(reinterpret_cast<const unsigned char *>(c), key.length(), nullptr);
}

