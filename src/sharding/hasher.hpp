//
// Created by realngnx on 11/06/2021.
//
#ifndef HASHER_H
#define HASHER_H

#include <iostream>


class Hasher {
public:
    virtual long to_md5_hash(std::string key) = 0;
    virtual long to_sha256_hash(std::string key) = 0;
};

class Digest: public Hasher {
public:
    Digest() = default;

    long to_md5_hash(std::string key) override;
    long to_sha256_hash(std::string key) override;

private:
    static long calculate_hash(char const* hashed);
    static unsigned char const* sha256(const std::string& key);
    static unsigned char const* md5(const std::string& key);

};


#endif //HASHER_H
