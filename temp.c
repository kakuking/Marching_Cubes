#include <assert.h>
#include <endian.h>
#include <stdint.h>
#include <stdio.h>

const char* fileName = "Skull.vol";
FILE* fp;

uint32_t readi32() {
    uint8_t buf[4];
    assert(fread(buf, 1, 4, fp) == 4);
    return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

float readf32() {
    uint32_t res = readi32();
    return *(float*) &res;
}

int main() {
    fp = fopen(fileName, "r");
    assert(fp != NULL);

    uint32_t numX = readi32();
    uint32_t numY = readi32();
    uint32_t numZ = readi32();

    uint32_t savedBorder = readi32();
    assert(savedBorder == 0);

    float trueSizeX = readf32();
    float trueSizeY = readf32();
    float trueSizeZ = readf32();

    uint8_t buf[numX][numY][numZ];
    assert(fread(buf, 1, numX * numY * numZ, fp) == numX * numY * numZ);

    printf("%u %u %u\n", numX, numY, numZ);
    // for (int x = 0; x < numX; x++) {
    //     for (int y = 0; y < numY; y++) {
    //         for (int z = 0; z < numZ; z++) {
    //             // printf("%hhu ", buf[x][y][z]);
    //         }
    //     }
    // }
    printf("\n");
}