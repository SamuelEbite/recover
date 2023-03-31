#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define BLOCK_SIZE 512

int main(int argc, char *argv[]) {
    // Ensure that the user has provided the file name of the forensic image
    if (argc != 2) {
        printf("Usage: recover image\n");
        return 1;
    }

    // Open the forensic image file
    char *infile = argv[1];
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL) {
        printf("Could not open %s.\n", infile);
        return 2;
    }

    // Create a buffer to hold the data from the current block
    uint8_t buffer[BLOCK_SIZE];

    // Create a flag to keep track of whether we have found the start of a new JPEG
    bool found_jpeg = false;

    // Create a counter to keep track of the number of JPEGs we have recovered
    int jpeg_counter = 0;

    // Create a pointer to the output file
    FILE *outptr = NULL;

    // Read the blocks of the forensic image one by one
    while (fread(buffer, BLOCK_SIZE, 1, inptr) == 1) {
        // Check if this block marks the start of a new JPEG
        if (buffer[0] == 0xff && buffer[1] == 0xd8 && buffer[2] == 0xff && (buffer[3] & 0xf0) == 0xe0) {
            // If we have already found a JPEG, close the previous output file
            if (found_jpeg) {
                fclose(outptr);
            }

            // Update the flag to indicate that we have found a new JPEG
            found_jpeg = true;

            // Generate the name of the output file
            char filename[8];
            sprintf(filename, "%03d.jpg", jpeg_counter);

            // Open the output file
            outptr = fopen(filename, "w");
            if (outptr == NULL) {
                printf("Could not create %s.\n", filename);
                return 3;
            }

            // Increment the counter of recovered JPEGs
            jpeg_counter++;
        }

        // If we have found a JPEG, write the current block to the output file
        if (found_jpeg) {
            fwrite(buffer, BLOCK_SIZE, 1, outptr);
        }
    }

    // Close the input file
    fclose(inptr);

    // Close the last output file
    fclose(outptr);

    return 0;
}
