#include <stdio.h>                // Include standard I/O library
#include "encode.h"               // Include encoding function declarations and EncodeInfo
#include "types.h"                // Include custom type definitions (e.g., Status)
#include <string.h>               // Include string manipulation functions
#include "common.h"               // Include common macros (e.g., MAGIC_STRING)

/* Get the image size for BMP */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;                   // Variables to store width and height
    fseek(fptr_image, 18, SEEK_SET);     // Move file pointer to byte 18 (width)
    fread(&width, sizeof(int), 1, fptr_image); // Read width (4 bytes)
    fread(&height, sizeof(int), 1, fptr_image); // Read height (4 bytes)
    fseek(fptr_image, 0, SEEK_SET);      // Reset file pointer to beginning
    return width * height * 3;           // Calculate total capacity (3 bytes per pixel)
}

/* Get the size of a file in bytes */
uint get_file_size(FILE *fptr)
{
    char ch;                              // Variable to read each character
    int count = 0;                         // Initialize counter
    while ((ch = fgetc(fptr)) != EOF)     // Read each byte until EOF
        count++;                           // Increment counter
    fseek(fptr, 0, SEEK_SET);             // Reset file pointer to beginning
    return count;                          // Return total size
}

/* Validate input and output file arguments */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char* source = strrchr(argv[2], '.'); // Find last '.' in source image name
    if (source == NULL || strcmp(source, ".bmp") != 0) // Check if extension is .bmp
        return e_failure;                  // Return failure if invalid
    encInfo->src_image_fname = argv[2];   // Store source image file name

    char* secret = strrchr(argv[3], '.'); // Find last '.' in secret file name
    if (secret == NULL || (strcmp(secret, ".txt") != 0 &&
                        strcmp(secret, ".c") != 0 &&
                        strcmp(secret, ".csv") != 0&&
                         strcmp(secret, ".sh") != 0)) // Validate secret file extension
        return e_failure;                  // Return failure if invalid
    encInfo->secret_fname = argv[3];       // Store secret file name

    if (argv[4] == NULL)                   // Check if output file not provided
        encInfo->stego_image_fname = "default.bmp"; // Use default stego file name
    else
    {
        char* des = strrchr(argv[4], '.'); // Find last '.' in output file name
        if (des == NULL || strcmp(des, ".bmp") != 0) // Check if extension is .bmp
            return e_failure;              // Return failure if invalid
        encInfo->stego_image_fname = argv[4]; // Store output stego file name
    }

    return e_success;                       // Return success if all valid
}

/* Open source, secret, and output files */
Status open_files(EncodeInfo *encInfo)
{
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb"); // Open source image in binary read mode
    if (!encInfo->fptr_src_image) { perror("fopen"); return e_failure; } // Error check

    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");       // Open secret file in read mode
    if (!encInfo->fptr_secret) { perror("fopen"); return e_failure; } // Error check

    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb"); // Open stego file in write binary mode
    if (!encInfo->fptr_stego_image) { perror("fopen"); return e_failure; } // Error check

    return e_success;                       // Return success if all files opened
}

/* Check if BMP has enough capacity for secret data */
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image); // Get image capacity
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);           // Get secret file size
    char *extn = strrchr(encInfo->secret_fname, '.');                           // Get secret file extension
    strcpy(encInfo->extn_secret_file, extn);                                    // Store extension

    uint total_required_bytes = 54 + ((encInfo->size_secret_file 
                                      + 4                   // Secret file size
                                      + strlen(encInfo->extn_secret_file) // Extension
                                      + 4                   // Extension size
                                      + strlen(MAGIC_STRING)) * 8); // Magic string in bits

    if (encInfo->image_capacity < total_required_bytes) // Check capacity
        return e_failure;                   // Return failure if insufficient
    return e_success;                        // Return success
}

/* Copy first 54-byte BMP header from source to stego image */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_stego_image)
{
    unsigned char header[54];                // Buffer to store BMP header
    size_t bytesRead = fread(header, 54, 1, fptr_src_image);  // Read header
    size_t bytesWritten = fwrite(header, 54, 1, fptr_stego_image); // Write header
    return (bytesWritten != bytesRead) ? e_failure : e_success; // Check success
}

/* Encode a byte into LSBs of 8 image bytes */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)            // Loop over 8 bits
    {
        int bit = (data >> (7 - i)) & 1;    // Extract i-th bit from data
        int clear = image_buffer[i] & ~1;   // Clear LSB of image byte
        clear |= bit;                        // Set LSB to data bit
        image_buffer[i] = clear;             // Store modified byte back
    }
    return e_success;                        // Return success
}

/* Encode 32-bit integer into LSBs of 32 bytes */
Status encode_size_to_lsb(int size, char *image_buffer)
{
    for (int i = 0; i < 32; i++)           // Loop over 32 bits
    {
        int bit = (size >> (31 - i)) & 1;   // Extract i-th bit
        int clear = image_buffer[i] & ~1;   // Clear LSB
        clear |= bit;                        // Set LSB
        image_buffer[i] = clear;             // Store back
    }
    return e_success;
}

/* Encode magic string into image */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char image_buffer[8];                    // Buffer for 8 bytes
    size_t bytesRead, bytesWritten;

    for (int i = 0; magic_string[i] != '\0'; i++)  // Loop through string
    {
        bytesRead = fread(image_buffer, 8, 1, encInfo->fptr_src_image);   // Read 8 bytes
        encode_byte_to_lsb(magic_string[i], image_buffer);                 // Encode char
        bytesWritten = fwrite(image_buffer, 8, 1, encInfo->fptr_stego_image); // Write modified bytes
        if (bytesWritten != bytesRead) return e_failure;                    // Check success
    }
    return e_success;
}

/* Encode secret file extension size */
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char image_buffer[32];                   // Buffer for 32 bytes
    size_t bytesRead, bytesWritten;

    bytesRead = fread(image_buffer, 32, 1, encInfo->fptr_src_image); // Read 32 bytes
    encode_size_to_lsb(size, image_buffer);                           // Encode size
    bytesWritten = fwrite(image_buffer, 32, 1, encInfo->fptr_stego_image); // Write modified bytes
    if (bytesWritten != bytesRead) return e_failure;                  // Check success
    return e_success;
}

/* Encode secret file extension string */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char image_buffer[8];                     // Buffer for 8 bytes
    size_t bytesRead, bytesWritten;

    for (int i = 0; file_extn[i] != '\0'; i++) // Loop through extension characters
    {
        bytesRead = fread(image_buffer, 8, 1, encInfo->fptr_src_image); // Read 8 bytes
        encode_byte_to_lsb(file_extn[i], image_buffer);                  // Encode char
        bytesWritten = fwrite(image_buffer, 8, 1, encInfo->fptr_stego_image); // Write modified bytes
        if (bytesWritten != bytesRead) return e_failure;                 // Check success
    }
    return e_success;
}
/* Encode secret file size into 32 bytes of LSBs */
Status encode_secret_file_size(int file_size, EncodeInfo *encInfo)
{
    char image_buffer[32];                     // Buffer for 32 bytes
    size_t bytesRead, bytesWritten;

    bytesRead = fread(image_buffer, 32, 1, encInfo->fptr_src_image); // Read 32 bytes from source
    encode_size_to_lsb(file_size, image_buffer);                      // Encode file size into LSBs
    bytesWritten = fwrite(image_buffer, 32, 1, encInfo->fptr_stego_image); // Write modified bytes
    if (bytesWritten != bytesRead) return e_failure;                  // Return failure if write mismatch
    return e_success;                                                  // Return success
}

/* Encode secret file data into image */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char image_buffer[8];                     // Buffer for 8 bytes per secret character
    size_t bytesRead, bytesWritten;

    for (int i = 0; i < encInfo->size_secret_file; i++) // Loop through all secret bytes
    {
        bytesRead = fread(image_buffer, 8, 1, encInfo->fptr_src_image);      // Read 8 bytes from image
        encode_byte_to_lsb(encInfo->secret_data[i], image_buffer);           // Encode one secret byte
        bytesWritten = fwrite(image_buffer, 8, 1, encInfo->fptr_stego_image); // Write modified bytes
        if (bytesWritten != bytesRead) return e_failure;                      // Check for write error
    }
    return e_success;                                                         // Return success
}

/* Copy remaining image data after encoding */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char buffer[1024];                         // Buffer for bulk copy
    size_t bytesRead, bytesWritten;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), fptr_src)) > 0) // Read in chunks
    {
        bytesWritten = fwrite(buffer, 1, bytesRead, fptr_dest);           // Write to destination
        if (bytesWritten != bytesRead) return e_failure;                   // Check for write mismatch
    }
    return e_success;                                                     // Return success
}

/* Main encoding driver function */
Status do_encoding(EncodeInfo *encInfo)
{
    Status res = open_files(encInfo);                // Open all necessary files
    if (res == e_failure) { printf("Error: File does not exist!\n"); return e_failure; }

    res = check_capacity(encInfo);                  // Verify image can hold secret
    if (res == e_failure) { printf("Error: Image file size should be greater than the secret file size!\n"); return e_failure; }

    res = copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image); // Copy BMP header
    if (res == e_failure) { printf("Error: Header file does not store in output image file!\n"); return e_failure; }
    else printf("Header file stored successfully!\n");

    res = encode_magic_string(MAGIC_STRING, encInfo); // Encode magic string
    if (res == e_failure) { printf("Error: Failed to encode magic string!\n"); return e_failure; }
    else printf("Magic string encoded successfully.\n");

    res = encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo); // Encode extension size
    if (res == e_failure) { printf("Error: Failed to encode secret file extn size!\n"); return e_failure; }
    else printf("Secret file extn size encoded successfully.\n");

    res = encode_secret_file_extn(encInfo->extn_secret_file, encInfo); // Encode extension string
    if (res == e_failure) { printf("Error: Failed to encode secret file extn!\n"); return e_failure; }
    else printf("Secret file extn encoded successfully.\n");

    res = encode_secret_file_size(encInfo->size_secret_file, encInfo); // Encode secret file size
    if (res == e_failure) { printf("Error: Failed to encode secret file size!\n"); return e_failure; }
    else printf("Secret file size encoded successfully.\n");

    fread(encInfo->secret_data, encInfo->size_secret_file, 1, encInfo->fptr_secret); // Read secret data into memory
    fseek(encInfo->fptr_secret, 0, SEEK_SET);                                         // Reset secret file pointer

    res = encode_secret_file_data(encInfo); // Encode secret file data
    if (res == e_failure) { printf("Error: Failed to encode secret file data!\n"); return e_failure; }
    else printf("Secret file data encoded successfully.\n");

    res = copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image); // Copy remaining image bytes
    if (res == e_failure) { printf("Error: Failed to encode remaining image data!\n"); return e_failure; }
    else printf("Remaining image data encoded successfully.\n");

    if (encInfo->fptr_src_image) fclose(encInfo->fptr_src_image);         // Close source image
    if (encInfo->fptr_secret) fclose(encInfo->fptr_secret);               // Close secret file
    if (encInfo->fptr_stego_image) fclose(encInfo->fptr_stego_image);     // Close stego image

    return e_success;                                                     // Return success after all steps
}
