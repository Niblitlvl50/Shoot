
#include "gtest/gtest.h"

#include "huffandpuff/huffman.h"
#include "Util/Random.h"

#include <vector>

TEST(Huffandpuff, Huffman)
{
    using byte = unsigned char;
    
    unsigned char huffbuf[HUFFHEAP_SIZE];
    
    std::vector<byte> input_bytes;
    input_bytes.reserve(1024);

    for(size_t index = 0; index < 1024; ++index)
        input_bytes.push_back(mono::RandomInt(0, 255));

    std::vector<byte> output_bytes;
    output_bytes.resize(2048, '\0');

    unsigned long output_size = huffman_compress(input_bytes.data(), input_bytes.size(), output_bytes.data(), output_bytes.size(), huffbuf);
    ASSERT_FALSE(output_size == 0ul);
    output_bytes.resize(output_size);

    std::vector<byte> decoded_bytes;
    decoded_bytes.resize(1024);

    unsigned long decoded_size = huffman_decompress(output_bytes.data(), output_bytes.size(), decoded_bytes.data(), decoded_bytes.size(), huffbuf);
    ASSERT_FALSE(decoded_size == 0ul);

    decoded_bytes.resize(decoded_size);
    ASSERT_TRUE(input_bytes == decoded_bytes);

    std::printf("Compressed size: %lu, Decompressed size: %lu\n", output_size, decoded_size);
}
