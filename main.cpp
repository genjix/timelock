#include "sha256.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

template<size_t Size> using byte_array = std::array<uint8_t, Size>;
constexpr size_t hash_size = 32;
typedef byte_array<hash_size> hash_digest;

typedef std::vector<uint8_t> data_chunk;

#define VERIFY_UNSIGNED(T) static_assert(std::is_unsigned<T>::value, \
    "The endian functions only work on unsigned types")

template <typename T>
byte_array<sizeof(T)> to_little_endian(T n)
{
    VERIFY_UNSIGNED(T);
    byte_array<sizeof(T)> out;
    for (auto i = out.begin(); i != out.end(); ++i)
    {
        *i = static_cast<uint8_t>(n);
        n >>= 8;
    }
    return out;
}

#undef VERIFY_UNSIGNED

data_chunk decode_hex(std::string hex)
{
    // Trim the fat.
    boost::algorithm::trim(hex);
    data_chunk result(hex.size() / 2);
    for (size_t i = 0; i + 1 < hex.size(); i += 2)
    {
        assert(hex.size() - i >= 2);
        auto byte_begin = hex.begin() + i;
        auto byte_end = hex.begin() + i + 2;
        // Perform conversion.
        int val = -1;
        std::stringstream converter;
        converter << std::hex << std::string(byte_begin, byte_end);
        converter >> val;
        if (val == -1)
            return data_chunk();
        assert(val <= 0xff);
        // Set byte.
        result[i / 2] = val;
    }
    return result;
}

template <typename T>
std::string encode_hex(T data)
{
    std::stringstream ss;
    ss << std::hex;
    for (int val: data)
        ss << std::setw(2) << std::setfill('0') << val;
    return ss.str();
}

template <typename D, typename T>
void extend_data(D& data, const T& other)
{
    data.insert(std::end(data), std::begin(other), std::end(other));
}

template <typename Data>
hash_digest sha256_hash(Data& chunk)
{
    hash_digest hash;
    SHA256__(chunk.data(), chunk.size(), hash.data());
    return hash;
}

template <typename Data>
void sha256_hash_nocopy(Data& chunk)
{
    SHA256__(chunk.data(), chunk.size(), chunk.data());
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: timelock NONCE N" << std::endl;
        return -1;
    }
    const std::string nonce_str = argv[1];
    const std::string n_str = argv[2];
    uint64_t number_iterations = 0;
    try
    {
        number_iterations = boost::lexical_cast<uint64_t>(n_str);
    }
    catch (const boost::bad_lexical_cast&)
    {
        std::cerr << "Error: bad N provided." << std::endl;
        return -1;
    }
    data_chunk data = decode_hex(nonce_str);
    if (data.empty())
    {
        std::cerr << "Error: bad NONCE provided." << std::endl;
        return -1;
    }
    if (data.size() != 32)
    {
        std::cerr << "Error: bad size for NONCE. Should be 32 bytes."
            << std::endl;
        return -1;
    }
    // Do one iteration into a hash.
    hash_digest result = sha256_hash(data);
    data.clear();
    // Now do the rest on the hash.
    for (size_t i = 1; i < number_iterations; ++i)
        sha256_hash_nocopy(result);
    // Display the result.
    std::cout << encode_hex(result) << std::endl;
    return 0;
}

