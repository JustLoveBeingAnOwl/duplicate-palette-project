#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>


// TODO:
// Make code check if image size is big enough for message.
// Add case for if colors in palette are less than required.
// Add case for if colors in palette are not all in the lowest indices.
// Store message length and bits per pixel somewhere in stego image.
// Check for row padding (?)


// Function: readBitmapFile
// Input: fileName, fileSize
//
// Description: Reads and outputs the contents of the provided bitmap file in hex.
unsigned char *readBitmapFile(char *fileName, unsigned int *fileSize)
{
    FILE *ptrFile;
    unsigned char *pFile;

    ptrFile = fopen(fileName, "rb"); // Specify read only and binary (no CR/LF added).

    if (ptrFile == NULL)
    {
        printf("Error opening file. \n");
        exit(1);
    }

    fseek(ptrFile, 0, SEEK_END);
    *fileSize = ftell(ptrFile);
    fseek(ptrFile, 0, SEEK_SET);

    // Malloc memory to hold the file, include room for the header and color table.
    pFile = malloc(*fileSize);

    if (pFile == NULL)
    {
        printf("Memory allocation failed. \n");
        exit(1);
    }

    // Read in complete file.
	// Buffer for data, size of each item, max # items, ptr to the file.
    fread(pFile, 1, *fileSize, ptrFile);
    fclose(ptrFile);

    return pFile;
}


// Function: readTextFile
// Input: fileName, fileSize
//
// Description: Reads and outputs the contents of the provided text file.
unsigned char *readTextFile(char *fileName, unsigned int *fileSize)
{
    FILE *ptrFile;
    unsigned char *pFile;

    ptrFile = fopen(fileName, "r"); // Specify read only.

    if (ptrFile == NULL)
    {
        printf("Error opening file. \n");
        exit(1);
    }

    fseek(ptrFile, 0, SEEK_END);
    *fileSize = ftell(ptrFile);
    fseek(ptrFile, 0, SEEK_SET);

    // Malloc memory to hold the file, include room for the header and color table.
    pFile = malloc(*fileSize);

    if (pFile == NULL)
    {
        printf("Memory allocation failed. \n");
        exit(1);
    }

    fread(pFile, 1, *fileSize, ptrFile);
    fclose(ptrFile);

    return pFile;
}


// Function: writeBitmapFile
// Input: fileName, bmpFile, bmpFileSize
//
// Description: Writes and outputs the contents of the provided char array to a file.
void writeBitmapFile (const char *filename, unsigned char *bmpFile, unsigned int bmpFileSize)
{
    FILE *file = fopen(filename, "wb");

    if (file == NULL)
    {
        printf("Error creating bitmap file. \n\n");
        exit(1);
    }

    fwrite(bmpFile, 1, bmpFileSize, file);
    fclose(file);
}


// Function: encryptBitmapFile
// Input: inputFileName, bmpFile, bmpFileSize, txtFile, txtFileSize, selectedBitsPerPixel
//
// Description: Encrypts the provided bitmap image with the secret message provided in the .txt file.
unsigned char *encryptBitmapFile(char *inputFileName, char* bmpFile, unsigned int* bmpFileSize, char* txtFile, unsigned int* txtFileSize, unsigned int* selectedBitsPerPixel)
{
    // Variable declarations.
    unsigned int bitCount = 0;
    unsigned int bitsPerPixel = 0;
    unsigned int paletteCount = 0;
    unsigned char pixelData;
    uint32_t dibHeaderSize = *(uint32_t *)(bmpFile + 14);
    int paletteIndex = 14 + dibHeaderSize;
    int encryptedPaletteIndex = 14 + dibHeaderSize;
    uint32_t pixelDataIndex = *(uint32_t *)(bmpFile + 10);

    // Step 1: Convert each character from text file into eight bits.
    unsigned int arraySize = (*txtFileSize) * 8;
    unsigned char* bitArray;

    // Malloc memory to hold bit array.
    bitArray = malloc(arraySize);

    if (bitArray == NULL)
    {
        printf("Memory allocation failed. \n");
        exit(1);
    }

    // Get bits from ASCII characters and place them in bitArray.
    for ( int i = 0; i < *txtFileSize; i++ )
    {
        unsigned char byte = txtFile[i];

        for ( int bit = 7; bit >= 0; bit-- )
        {
            int value = ( byte >> bit ) & 1;
            bitArray[bitCount] = value;
            bitCount++;
        }
    }

    // Step 2: Have user input how many bits per pixel they would like to hide in the bitmap image.
    printf("\n\n How many bits per pixel would you like to hide in the bitmap image? 1, 2, 3, or 4? \n\n");
    scanf("%u", &bitsPerPixel);

    if ( bitsPerPixel != 1 && bitsPerPixel != 2 && bitsPerPixel != 3 && bitsPerPixel != 4 )
    {
        printf("Unacceptable input. Please input either 1, 2, 3, or 4.");
        exit(1);
    }
    *selectedBitsPerPixel = bitsPerPixel;

    // Step 3: Use color reducer if 24-bit bitmap image provided, compression is used, or if more empty palette entries are required.
    // Number of colors needed after reduction.
    uint32_t colorCount = 256 >> bitsPerPixel;

    // Check bitmap format.
    uint32_t bpp = *(uint16_t *)(bmpFile + 28);
    uint32_t compression = *(uint32_t *)(bmpFile + 30);

    // Check if more empty palette entries are required.
    uint32_t colorsUsed = *(uint32_t *)(bmpFile + 46);

    if (colorsUsed == 0)
    {
        colorsUsed = 256;
    }

    // Reduce the image to an 8-bit bitmap image with the correct amount of colors.
    if (bpp != 8 || compression != 0 || colorsUsed != colorCount)
    {
        printf("Reducing image to %u colors...\n", colorCount);

        char command[512];
        snprintf(command, sizeof(command), "octree.exe \"%s\" reduced.bmp %u", inputFileName, colorCount);

        if (system(command) != 0)
        {
            printf("Conversion failed.\n");
            exit(1);
        }

        bmpFile = readBitmapFile("reduced.bmp", bmpFileSize);

        dibHeaderSize = *(uint32_t *)(bmpFile + 14);
        paletteIndex = 14 + dibHeaderSize;
        encryptedPaletteIndex = 14 + dibHeaderSize;
        pixelDataIndex = *(uint32_t *)(bmpFile + 10);
    }

    // Step 4: Create char array that will be the encrypted bitmap file.
    unsigned char *encryptedFile;

    // Malloc memory to hold the file, include room for the header and color table.
    encryptedFile = malloc(*bmpFileSize);

    if (encryptedFile == NULL)
    {
        printf("Memory allocation failed. \n");
        exit(1);
    }

    memcpy(encryptedFile, bmpFile, *bmpFileSize);

    // Step 5: Stretch palette based on amount of bits selected.
    int duplicateAmount = 1 << bitsPerPixel;
    paletteCount = 256 / duplicateAmount;

    for ( int j = 0; j < paletteCount; j++ )
    {
        for ( int copy = 0; copy < duplicateAmount; copy++ )
        {
            memcpy(&encryptedFile[encryptedPaletteIndex], &bmpFile[paletteIndex], 4);
            encryptedPaletteIndex += 4;
        }
        paletteIndex += 4;
    }
    *(uint32_t *)(encryptedFile + 46) = 256;

    // Step 6: Change pixel data entries to accommodate stretched palette.
    uint32_t k = *(uint32_t *)(bmpFile + 10);
    while ( k < *bmpFileSize )
    {
        encryptedFile[k] <<= bitsPerPixel;
        k++;
    }


    // Step 7: Change colors at pixel data to start encrypting secret message.
    unsigned int messageBitIndex = 0;
    unsigned int mask = (1u << bitsPerPixel) - 1;

    while (messageBitIndex < arraySize)
    {
        unsigned int hiddenValue = 0;

        for (unsigned int bit = 0; bit < bitsPerPixel; bit++)
        {
            hiddenValue <<= 1;

            if (messageBitIndex < arraySize)
            {
                hiddenValue |= bitArray[messageBitIndex];
                messageBitIndex++;
            }
        }

        encryptedFile[pixelDataIndex] =
            (encryptedFile[pixelDataIndex] & ~mask) | hiddenValue;

        pixelDataIndex++;
    }

    // Step 8: Return new file with hidden image.
    free(bitArray);

    return encryptedFile;
}


/* Function: extractMessage
// Input: pixels, pixelCount, bitsPerPixel
//
// Description: Extracts message from least significant bits of pixel data.
void extractMessage(unsigned char *pixels,
                    int pixelCount,
                    int bitsPerPixel)
{
    printf("=====================================\n");
    printf("Inside extractMessage()\n");
    printf("Pixels to process : %d\n", pixelCount);
    printf("Bits per pixel    : %d\n", bitsPerPixel);
    printf("=====================================\n\n");

    // Create a mask based on the user's selection.
    // 1 bit -> 00000001
    // 2 bits -> 00000011
    // 3 bits -> 00000111
    // 4 bits -> 00001111

    int mask = (1 << bitsPerPixel) - 1;

    printf("Mask = 0x%X\n\n", mask);

    // Bit buffer used to rebuild bytes
    uint32_t bitBuffer = 0;
    int bitsInBuffer = 0;

    int recoveredBytes = 0;

    printf("Recovered Bytes\n");
    printf("------------------------------\n");

    for (int i = 0; i < pixelCount; i++)
    {
        // Extract the hidden bits from this palette index
        unsigned int hiddenBits = pixels[i] & mask;

        // Add them to our buffer
        bitBuffer = (bitBuffer << bitsPerPixel) | hiddenBits;
        bitsInBuffer += bitsPerPixel;

        // Whenever we have at least one byte ready
        while (bitsInBuffer >= 8)
        {
            bitsInBuffer -= 8;

            unsigned char recoveredByte =
                (bitBuffer >> bitsInBuffer) & 0xFF;

            printf("%02X", recoveredByte);

            if (recoveredByte >= 32 && recoveredByte <= 126)
            {
                printf("    '%c'", recoveredByte);
            }

            printf("\n");

            recoveredBytes++;
        }
    }

    printf("\n");
    printf("Extraction complete.\n");
    printf("Recovered %d byte(s).\n", recoveredBytes);
}
*/


// Main.
int main(int argc, char* argv[])
{
    // Tells user how to use application if no arguments given.
    if ( argc != 4 )
    {
        printf("\n\n To encrypt a bitmap image: \n\n");
        printf("embed.exe <inputfile.bmp> <secretmessage.txt> <outputfile.bmp> \n\n");
        exit(1);
    }

    // Gets data from bitmap image, where each element in the array bmpFile is one byte in hex.
    unsigned int bmpFileSize;
    unsigned char *bmpFile = readBitmapFile(argv[1], &bmpFileSize);

    // Gets each character from text file, where each element in the array txtFile is one ASCII character.
    unsigned int txtFileSize;
    unsigned char *txtFile = readTextFile(argv[2], &txtFileSize);

    // Gets stego image data.
    unsigned int bitsPerPixel;
    unsigned char *encryptedFile = encryptBitmapFile(argv[1], bmpFile, &bmpFileSize, txtFile, &txtFileSize, &bitsPerPixel);

    // Creates stego image file.
    writeBitmapFile(argv[3], encryptedFile, bmpFileSize);

    /* Extract message from stego image.
    int pixelOffset = *(uint32_t *)(encryptedFile + 10);
    extractMessage(encryptedFile + pixelOffset, bmpFileSize - pixelOffset, bitsPerPixel); */

    // Free memory.
    free(bmpFile);
    free(txtFile);
    free(encryptedFile);


    return 0;
}
