/*
CS4463 - Steganography Class
Project Members: Matthew M, Hannah M, and Evan H.

This project for our steganography class is designed to test the "Duplicate Palette Approach" using the following parameters:
- The Duplicate Palette approach only works if the number of colors used is less than or equal to 128.
If the number of colors used is greater than 128, we need to reduce it to 128 or less. If it is less than 128, we can just use it as is.
- Our implementation should allow the user to select between 1, 2, 3, or 4 embedding bits per pixel. 
Supporting multiple embedding levels provides flexibility and allows us to evaluate the trade-offs between hiding capacity, visual quality, and detectability.
- We will be using the interleaving approach for duplicate palette entries.
This method makes the embedded message correspond to the least significant bits of the palette index and builds naturally on the LSB-based techniques that we covered throughout the course. 
- We plan to compare different embedding levels using embedding capacity, image quality, extraction accuracy, and any observable statistical changes. 
This comparison will demonstrate the trade-offs between capacity and detectability.
*/

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
