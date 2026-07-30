#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>


// Function: readBitmapFile
// Input: fileName, fileSize
//
// Description: Reads and outputs the contents of the provided bitmap file in hex.
unsigned char *readBitmapFile(char *fileName, unsigned int *fileSize)
{
    FILE *file;
    unsigned char *bmpFile;

    file = fopen(fileName, "rb"); // Specify read only and binary (no CR/LF added).

    if (file == NULL)
    {
        printf("Error opening file.\n");
        return 0;
    }

    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Malloc memory to hold the file, include room for the header and color table.
    bmpFile = malloc(*fileSize);

    if (bmpFile == NULL)
    {
        printf("Memory allocation failed.\n");
        return 0;
    }

    // Read in complete file.
	// Buffer for data, size of each item, max # items, ptr to the file.
    fread(bmpFile, 1, *fileSize, file);
    fclose(file);

    return bmpFile;
}


// Function: readTextFile
// Input: fileName, fileSize
//
// Description: Reads and outputs the contents of the provided text file.
unsigned char *readTextFile(char *fileName, unsigned int *fileSize)
{
    FILE *file;
    unsigned char *txtFile;

    file = fopen(fileName, "r"); // Specify read only.

    if (file == NULL)
    {
        printf("Error opening file.\n");
        return 0;
    }

    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Malloc memory to hold the file, include room for the header and color table.
    txtFile = malloc(*fileSize);

    if (txtFile == NULL)
    {
        printf("Memory allocation failed.\n");
        return 0;
    }

    fread(txtFile, 1, *fileSize, file);
    fclose(file);

    return txtFile;
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
        printf("Error creating bitmap file.\n\n");
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
    uint32_t colorsUsed = 0;
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
        printf("Memory allocation failed.\n");
        return 0;
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
    printf("How many bits per pixel would you like to hide in the bitmap image? 1, 2, 3, or 4? \n\n");
    scanf("%u", &bitsPerPixel);
    printf("\n");

    if ( bitsPerPixel != 1 && bitsPerPixel != 2 && bitsPerPixel != 3 && bitsPerPixel != 4 )
    {
        printf("Unacceptable input. Please input either 1, 2, 3, or 4.\n");
        return 0;
    }
    *selectedBitsPerPixel = bitsPerPixel;

    // Step 3: Use color reducer if 24-bit bitmap image provided, compression is used, or if more empty palette entries are required.
    // Number of colors needed after reduction.
    uint32_t colorCount = 256 >> bitsPerPixel;

    // Check bitmap format.
    uint32_t bpp = *(uint16_t *)(bmpFile + 28);
    uint32_t compression = *(uint32_t *)(bmpFile + 30);

    // Check if more empty palette entries are required.
    colorsUsed = *(uint32_t *)(bmpFile + 46);

    if (colorsUsed == 0)
    {
        colorsUsed = 256;
    }

    // Reduce the image to an 8-bit bitmap image with the correct amount of colors.
    if (bpp != 8 || compression != 0 || colorsUsed > colorCount)
    {
        printf("This image must have its colors reduced to encrypt.\n");
        printf("A copy of this image can be created to use for encryption.\n");
        printf("If you would like to color reduce and encrypt a copy of this image,\n");
        printf("type \"y\". Original image will be preserved.\n");
        printf("type any other character if you do not.\n\n");
        char yesOrNo;
        scanf(" %c", &yesOrNo);
        printf("\n");

        if ( yesOrNo != 'y' && yesOrNo != 'Y' )
        {
            printf("Color reduction declined. Ending program.\n");
            return 0;
        }

        printf("Reducing image to %u colors...\n", colorCount);

        char command[512];
        snprintf(command, sizeof(command), "octree.exe \"%s\" reduced.bmp %u", inputFileName, colorCount);

        if (system(command) != 0)
        {
            printf("Conversion failed.\n");
            exit(1);
        }

        bmpFile = readBitmapFile("reduced.bmp", bmpFileSize);

        colorsUsed = *(uint32_t *)(bmpFile + 46);
        dibHeaderSize = *(uint32_t *)(bmpFile + 14);
        paletteIndex = 14 + dibHeaderSize;
        encryptedPaletteIndex = 14 + dibHeaderSize;
        pixelDataIndex = *(uint32_t *)(bmpFile + 10);

        if (colorsUsed == 0)
        {
            colorsUsed = 256;
        }
    }

    // Step 4: Create char array that will be the encrypted bitmap file.
    unsigned char *encryptedFile;

    // Malloc memory to hold the file, include room for the header and color table.
    encryptedFile = malloc(*bmpFileSize);

    if (encryptedFile == NULL)
    {
        printf("Memory allocation failed.\n");
        return 0;
    }

    memcpy(encryptedFile, bmpFile, *bmpFileSize);

    // Step 5: Stretch palette based on amount of bits selected.
    int duplicateAmount = 1 << bitsPerPixel;

    for ( int j = 0; j < colorsUsed; j++ )
    {
        for ( int copy = 0; copy < duplicateAmount; copy++ )
        {
            memcpy(&encryptedFile[encryptedPaletteIndex], &bmpFile[paletteIndex], 4);
            encryptedPaletteIndex += 4;
        }
        paletteIndex += 4;
    }
    *(uint32_t *)(encryptedFile + 46) = colorsUsed * duplicateAmount;

    // Step 6: Check if message is too long for bitmap image.
    uint32_t pixelDataOffset = *(uint32_t *)(bmpFile + 10);
    uint32_t availablePixels = *bmpFileSize - pixelDataOffset;
    unsigned int requiredPixels = (arraySize + bitsPerPixel - 1) / bitsPerPixel;

    if ( availablePixels < requiredPixels )
    {
        printf("Message too large to be encrypted in this image.\n");
        free(bitArray);
        free(encryptedFile);
        return 0;
    }

    // Step 7: Change pixel data entries to accommodate stretched palette.
    uint32_t k = *(uint32_t *)(bmpFile + 10);
    while ( k < *bmpFileSize )
    {
        encryptedFile[k] <<= bitsPerPixel;
        k++;
    }


    // Step 8: Change colors at pixel data to start encrypting secret message.
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

    // Step 9: Return new file with hidden image.
    free(bitArray);

    return encryptedFile;
}
