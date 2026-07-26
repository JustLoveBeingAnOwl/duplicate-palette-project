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
#include <math.h>

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
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BmpInfoHeader;

#pragma pack(pop)

int main(int argc, char *argv[])
{
    // check correct number of arguments
    if (argc != 3) {
        fprintf(stderr, "Not Enough Arguments! Need: %s <bmp_file> <message_file>\n", argv[0]);
        return 1;
    }

    // Open the BMP file
    FILE *bmpfile = fopen(argv[1], "rb");
    if (!bmpfile) {
        perror("Failed to open file");
        return 1;
    }
    printf("Opened BMP file successfully.\n");

    // Open the BMP file and read its headers to get the following information that's important for our steganography implementation: 
    // - It's size
    // - It's # of colors.
    // We need to check how many colors it contains because if it is >128, we need to reduce it to 128 or less. If it is less than 128, we can just use it as is.
    // Read Headers
    BmpFileHeader fileHeader;
    BmpInfoHeader infoHeader;

    fread(&fileHeader, sizeof(BmpFileHeader), 1, bmpfile);
    fread(&infoHeader, sizeof(BmpInfoHeader), 1, bmpfile);

    //check if valid bmp file by looking at the header
    if (fileHeader.bfType != 0x4D42) {
        fprintf(stderr, "Not a valid BMP file.\n");
        fclose(bmpfile);
        return 1;
    }

    //before continuing, do some checks:
    // - is 8-bit bmp
    // - palette is less than or equal to 128 colors
    // for either, print a message you need to use a third-party software to convert the image to 8-bit and/or reduce the number of colors to 128 or less.
    if (infoHeader.biBitCount != 8) {
        fprintf(stderr, "bitCount: %d\n", infoHeader.biBitCount);
        fprintf(stderr, "This program only supports 8-bit BMP files. Please convert the image to 8-bit using a third-party software.\n");
        fclose(bmpfile);
        return 1;
    }
    printf("bitCount: %d\n", infoHeader.biBitCount);
    if (infoHeader.biClrUsed > 128) {
        fprintf(stderr, "biClrUsed: %d\n", infoHeader.biClrUsed);
        fprintf(stderr, "This program only supports BMP files with 128 or fewer colors. Please reduce the number of colors to 128 or less using a third-party software.\n");
        fclose(bmpfile);
        return 1;
    }
    printf("biClrUsed: %d\n", infoHeader.biClrUsed);
    //note, this doesn't truly check color count in the palette, it only checks the biClrUsed field in the header.
    // TODO: Implement a function to read the color palette and count the actual number of unique colors used in the image.

    // Calculate the size of the image data
    size_t data_size = infoHeader.biSizeImage;
    if (data_size == 0) {
        data_size = infoHeader.biWidth * infoHeader.biHeight;
    }
    
    // Calculate the size of the image data
    unsigned char* data = (unsigned char*)malloc(data_size);
    if (!data) {
        printf("Error: Memory allocation failed.\n");
        fclose(bmpfile);
        return 1;
    }

    // Jump to pixel data position and read it
    fseek(bmpfile, fileHeader.bfOffBits, SEEK_SET);
    fread(data, 1, data_size, bmpfile);

    
    // Open the txt file that will store our message.
    // We will take the message size and store it in the first few bytes of our carrier image
    // Then we will store the message itself in the remaining bytes of our carrier image.
    FILE *txtfile = fopen(argv[2], "rb");
    if (!txtfile) {
        perror("Failed to open the message file");
        fclose(bmpfile);
        return 1;
    }

    // Get its size
    fseek(txtfile, 0, SEEK_END);
    long txt_size = ftell(txtfile);
    rewind(txtfile);

    //allocate memory to store the message
    unsigned char *message = (char *)malloc(txt_size);
    if (!message) {
        fprintf(stderr, "Failed to allocate memory for the message.\n");
        fclose(txtfile);
        fclose(bmpfile);
        return 1;
    }

    // store the message in the allocated memory
    fread(message, 1, txt_size, txtfile);

    // Close the files and free the allocated memory
    fclose(txtfile);
    fclose(bmpfile);
    free(data);
    free(message);

    return 0;
}
