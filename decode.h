#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>          // Standard I/O functions
#include "types1.h"         // Custom type definitions (e.g., Status1)
#include "types.h"          // Additional type definitions
#include "common.h"         // Common macros and constants (e.g., MAGIC_STRING)

// Structure to hold all information required for decoding
typedef struct _DecodeInfo
{
    /* Stego image details */
    char *stego_image_fname;   // Stego (encoded) image file name
    FILE *fptr_stego_image;    // File pointer for stego image

    /* Output file details */
    char *output_fname;        // Output file name
    FILE *fptr_output_file;    // File pointer for decoded output file

    /* Decoded data */
    char extn_secret_file[10]; // Buffer to hold decoded secret file extension (e.g., .txt, .c)
    int size_secret_file;      // Size of the decoded secret file
} DecodeInfo;

/* Function declarations */

// Validate input file names and extensions for decoding
Status1 read_and_validate_decode_file(char* argv[], DecodeInfo *decInfo);

// Main driver function to perform full decoding
Status1 do_decoding(DecodeInfo *decInfo);

/* File handling functions */

// Open the stego image file for decoding
Status1 open_files_decode(DecodeInfo *decInfo);

// Skip the BMP header (first 54 bytes)
Status1 skip_bmp_header(FILE *fptr_stego_image);

/* LSB decoding functions */

// Decode a single byte from the least significant bits of 8 image bytes
char decode_byte_from_lsb(char *image_buffer);

// Decode a 32-bit integer (file size or extension size) from LSBs of 32 image bytes
int decode_size_from_lsb(char *image_buffer);

// Verify magic string in stego image
Status1 decode_magic_string(DecodeInfo *decInfo);

// Decode the size of the secret file extension
Status1 decode_secret_file_extn_size(DecodeInfo *decInfo, int *extn_size);

// Decode the actual secret file extension and prepare output file
Status1 decode_secret_file_extn(DecodeInfo *decInfo, int extn_size);

// Decode the size of the secret file
Status1 decode_secret_file_size(DecodeInfo *decInfo);

// Decode secret file data and write to output file
Status1 decode_secret_file_data(DecodeInfo *decInfo);

#endif
