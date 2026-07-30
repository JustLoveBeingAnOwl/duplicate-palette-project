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

#include "extract.c"
#include "embed.c"

#pragma pack(push, 1)

typedef struct
{
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BmpFileHeader;

typedef struct
{
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BmpInfoHeader;

#pragma pack(pop)

int main(int argc, char *argv[])
{
    // int value to check if the user has selected embedding or extraction. It should be either 1 or 2. End program if otherwise.
    int mode = atoi(argv[1]);

    if (mode != 1 && mode != 2)
    {
        fprintf(stderr, "Invalid mode! Please select either 1 for embedding or 2 for extraction.\n");
        return 1;
    }

    // Check correct number of arguments for embedding.
    if (mode == 1 && argc != 5)
    {
        fprintf(stderr, "Incorrect Number of Arguments! Need: %s <mode> <bmp_file> <message_file> <output_file>\n", argv[0]);
        return 1;
    }

    // Check correct number of arguments for extraction. We do not need the message file for extraction, so we only need 3 arguments.
    if (mode == 2 && argc != 3)
    {
        fprintf(stderr, "Incorrect Number of Arguments! Need: %s <mode> <bmp_file>\n", argv[0]);
        return 1;
    }

    // Open the BMP file.
    FILE *bmpfile = fopen(argv[2], "rb");
    if (!bmpfile)
    {
        perror("Failed to open file.\n");
        return 1;
    }

    printf("\nOpened BMP file successfully.\n\n");

    // Open the BMP file and read its headers to get the following information that's important for our steganography implementation:
    // - It's size
    // - It's # of colors.
    // We need to check how many colors it contains because if it is >128, we need to reduce it to 128 or less. If it is less than 128, we can just use it as is.
    // Read Headers.
    BmpFileHeader fileHeader;
    BmpInfoHeader infoHeader;

    fread(&fileHeader, sizeof(BmpFileHeader), 1, bmpfile);
    fread(&infoHeader, sizeof(BmpInfoHeader), 1, bmpfile);

    // Check if valid bmp file by looking at the header.
    if (fileHeader.bfType != 0x4D42)
    {
        fprintf(stderr, "Not a valid BMP file.\n");
        fclose(bmpfile);
        return 1;
    }

    printf("Image Information\n");
    printf("-----------------\n");
    printf("Width      : %d\n", infoHeader.biWidth);
    printf("Height     : %d\n", infoHeader.biHeight);
    printf("Bit Count  : %d\n", infoHeader.biBitCount);
    printf("Colors Used: %u\n", infoHeader.biClrUsed);
    printf("Image Size : %u\n\n", infoHeader.biSizeImage);

    size_t data_size = infoHeader.biSizeImage;

    if (data_size == 0)
    {
        data_size = infoHeader.biWidth * abs(infoHeader.biHeight);
    }

    // Calculate the size of the image data.
    unsigned char* data = (unsigned char*)malloc(data_size);
    if (!data)
    {
        printf("Error: Memory allocation failed.\n");
        fclose(bmpfile);
        return 1;
    }

    // Jump to pixel data position and read it.
    fseek(bmpfile, fileHeader.bfOffBits, SEEK_SET);
    fread(data, 1, data_size, bmpfile);

    // If mode == 1, embedding mode (1), we need to embed the message into the image. We will do this by calling the encryptBitmapFile() function.
    // If we are in extraction mode (2), we need to extract the message from the image. We will do this by calling the extractMessage() function.
    if (mode == 1 )
    {
        printf("-- Embedding mode selected --\n\n");

        // Get data from bitmap image, where each element in the array bmpFile is one byte in hex.
        unsigned int bmpFileSize;
        unsigned char *coverFile = readBitmapFile(argv[2], &bmpFileSize);

        // Get each character from text file, where each element in the array txtFile is one ASCII character.
        unsigned int txtFileSize;
        unsigned char *txtFile = readTextFile(argv[3], &txtFileSize);

        // Get stego image data.
        unsigned int bitsPerPixel;
        unsigned char *encryptedFile = encryptBitmapFile(argv[2], coverFile, &bmpFileSize, txtFile, &txtFileSize, &bitsPerPixel);

        // Create stego image file.
        writeBitmapFile(argv[4], encryptedFile, bmpFileSize);

        // Free memory.
        free(coverFile);
        free(txtFile);
        free(encryptedFile);
    }

    // Open the txt file that will store our message.
    // We will take the message size and store it in the first few bytes of our carrier image.
    // Then we will store the message itself in the remaining bytes of our carrier image.
    if (mode == 2)
    {
        printf("-- Extraction mode selected --\n\n");
        printf("Pixel data successfully loaded.\n\n");
        int bits;
        printf("Bits per pixel to extract (1-4): ");
        if (scanf("%d", &bits) != 1 || bits < 1 || bits > 4)
        {
            printf("Invalid selection.\n");
            free(data);
            return 1;
        }

        printf("\nCalling extractMessage()...\n\n");

        extractMessage(data,
                   (int)data_size,
                   bits);
    }

    // The bmp file is used for both embedding and extraction, so we can close it at the end of the program.
    fclose(bmpfile);
    free(data);

    printf("\nProgram finished successfully.\n");

    return 0;
}
