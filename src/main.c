#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>>

#pragma pack(push, 1)
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BmpFileHeader;

typedef struct {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed; //This is the main variable that tells us how many colors are used in the palette! If it is more than 128, we need to reduce it to 128 or less. If it is less than 128, we can just use it as is.
    uint32_t biClrImportant;
} BmpInfoHeader;

#pragma pack(pop)

int main()
{
    // Open the BMP file
    FILE *file = fopen("image.bmp", "rb");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }
    // For now we just want to make sure we can open the file and check:
    // - It's size
    // - It's # of colors.
    // We need to check how many colors it contains because if it is >128, we need to reduce it to 128 or less. If it is less than 128, we can just use it as is.
    // Read Headers
    BmpFileHeader fileHeader;
    BmpInfoHeader infoHeader;

    fread(&fileHeader, sizeof(BmpFileHeader), 1, file);
    fread(&infoHeader, sizeof(BmpInfoHeader), 1, file);

    // Close the BMP file
    fclose(file);

    return 0;
}
