/*
 * extract.c
 * Duplicate Palette Project
 * Hannah M. - Extraction Module
 */

#include <stdio.h>
#include <stdint.h>
#include "extract.h"

void extractMessage(unsigned char *pixels,
                    int pixelCount,
                    int bitsPerPixel)
{
    printf("=====================================\n");
    printf("Inside extractMessage()\n");
    printf("Pixels to process : %d\n", pixelCount);
    printf("Bits per pixel    : %d\n", bitsPerPixel);
    printf("=====================================\n\n");

    /* Create a mask based on the user's selection.
       1 bit -> 00000001
       2 bits -> 00000011
       3 bits -> 00000111
       4 bits -> 00001111
    */
    int mask = (1 << bitsPerPixel) - 1;

    printf("Mask = 0x%X\n\n", mask);

    /* Bit buffer used to rebuild bytes */
    uint32_t bitBuffer = 0;
    int bitsInBuffer = 0;

    int recoveredBytes = 0;

    printf("Recovered Bytes\n");
    printf("------------------------------\n");

    for (int i = 0; i < pixelCount; i++)
    {
        /* Extract the hidden bits from this palette index */
        unsigned int hiddenBits = pixels[i] & mask;

        /* Add them to our buffer */
        bitBuffer = (bitBuffer << bitsPerPixel) | hiddenBits;
        bitsInBuffer += bitsPerPixel;

        /* Whenever we have at least one byte ready */
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