#include <stdio.h>              // Standard I/O functions
#include "decode.h"             // Include header file for decode function declarations
#include "types1.h"             // Include custom type definitions (e.g., Status1)
#include <string.h>             // For string handling functions like strcmp, strrchr
#include "common.h"             // Include common definitions (e.g., MAGIC_STRING)

// Function to validate decoding input and output file extensions
Status1 read_and_validate_decode_file(char* argv[], DecodeInfo* decInfo)
{
    // Check if decoding source file (BMP) has a valid extension
    char* source = (strrchr(argv[2], '.'));       // Find the last '.' in the source file name
    if (source == NULL || strcmp(source, ".bmp") != 0)  // If no '.' or not a ".bmp" file
    {
        return d_failure;                         // Return failure if invalid
    }
    else
    {
        decInfo->stego_image_fname = argv[2];     // Store valid stego image file name
    }

    // Check if decoding output file (text or code) has a valid extension
    char* secret = strrchr(argv[3], '.');  // Find last '.' in output file

if (secret != NULL) {
    // If there is an extension, check if it is allowed
    if (strcmp(secret, ".txt") != 0 &&
        strcmp(secret, ".c") != 0 &&
        strcmp(secret, ".csv") != 0 &&
        strcmp(secret, ".sh") != 0) 
    {
        return d_failure;  // Invalid extension
    }
}

// If no extension, accept it as valid
decInfo->output_fname = argv[3];  // Store output filename
    
    return d_success;                             // Return success if both files are valid
}

// Function to open the stego (encoded) BMP image file for decoding
Status1 open_files_decode(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");  // Open stego image file in binary read mode

    if (decInfo->fptr_stego_image == NULL)  // Check if file failed to open
    {
        printf("Error: Cannot open stego image file %s\n", decInfo->stego_image_fname);  // Print error message
        return d_failure;  // Return failure status
    }

    return d_success;  // Return success if file opened successfully
}

// Function to skip the BMP header (first 54 bytes)
Status1 skip_bmp_header(FILE *fptr_stego_image)
{
    fseek(fptr_stego_image, 54, SEEK_SET);  // Move file pointer to byte 54 (start of image data)

    if (ftell(fptr_stego_image) != 54)  // Verify that the pointer moved correctly
    {
        return d_failure;  // Return failure if pointer not at expected position
    }

    return d_success;  // Return success if header skipped successfully
}


// Function to decode one byte of hidden data from the LSBs of 8 image bytes
char decode_byte_from_lsb(char *image_buffer)
{
    char data = 0;                            // Variable to store the decoded byte

    for (int i = 0; i < 8; i++)               // Loop through 8 bytes (8 bits of data)
    {
        int bit = image_buffer[i] & 1;        // Extract the least significant bit (LSB)
        data = (data << 1) | bit;             // Shift data left and add the extracted bit
    }

    return data;                              // Return the decoded character
}

// Function to decode and verify the magic string from the stego image
Status1 decode_magic_string(DecodeInfo *decInfo)
{
    char image_buffer[8];                                     // Buffer to hold 8 bytes from image
    char magic[strlen(MAGIC_STRING) + 1];                     // Buffer to store decoded magic string

    for (int i = 0; i < strlen(MAGIC_STRING); i++)            // Loop for each character in MAGIC_STRING
    {
        fread(image_buffer, 1, 8, decInfo->fptr_stego_image); // Read 8 bytes from stego image
        magic[i] = decode_byte_from_lsb(image_buffer);        // Decode one character from LSBs
    }
    
    magic[strlen(MAGIC_STRING)] = '\0';                       // Null-terminate the decoded string

    if (strcmp(magic, MAGIC_STRING) == 0)                     // Compare decoded string with original magic string
    {
        return d_success;                                     // Return success if it matches
    }

    return d_failure;                                         // Return failure if it doesn't match
}

// Function to decode a 4-byte (32-bit) integer size from LSBs of 32 image bytes
int decode_size_from_lsb(char* image_buffer)
{
    int data = 0;                            // Variable to store decoded size

    for (int i = 0; i < 32; i++)             // Loop through 32 bytes (32 bits)
    {
        int bit = image_buffer[i] & 1;       // Extract the LSB of each byte
        data = (data << 1) | bit;            // Shift data left and add the bit
    }

    return data;                             // Return the decoded integer size
}


// Function to decode the size of the secret file extension from the stego image
Status1 decode_secret_file_extn_size(DecodeInfo *decInfo, int *extn_size)
{
    char image_buffer[32];                                    // Buffer to hold 32 bytes (for 32 bits of size)
    size_t bytesRead;

    bytesRead = fread(image_buffer, 1, 32, decInfo->fptr_stego_image);  // Read 32 bytes from stego image

    if (bytesRead != 32)                                      // Check if 32 bytes were successfully read
    {
        return d_failure;                                     // Return failure if read error
    }

    *extn_size = decode_size_from_lsb(image_buffer);           // Decode 4-byte integer (extension size) from LSBs
    return d_success;                                          // Return success
}

// Function to decode the actual secret file extension (e.g., ".txt", ".c") from the stego image
Status1 decode_secret_file_extn(DecodeInfo *decInfo, int extn_size)
{
    char image_buffer[8];                                     // Buffer to hold 8 bytes (for one character)
    int i;

    for (i = 0; i < extn_size; i++)                           // Loop for each character of extension
    {
        fread(image_buffer, 1, 8, decInfo->fptr_stego_image); // Read 8 bytes from image for one character
        decInfo->extn_secret_file[i] = decode_byte_from_lsb(image_buffer);  // Decode character from LSBs
    }

    decInfo->extn_secret_file[i] = '\0';                      // Null-terminate the decoded extension string
   
    char* dot = strrchr(decInfo->output_fname, '.');           // Find the last '.' in output filename
    if (dot != NULL)                                          // If an extension already exists
    {
        *dot = '\0';                                          // Remove the old extension
    }

    strcat(decInfo->output_fname, decInfo->extn_secret_file);  // Append the decoded extension to output filename

    decInfo->fptr_output_file = fopen(decInfo->output_fname, "wb");  // Create output file in binary write mode
    if (decInfo->fptr_output_file == NULL)                    // Check if file creation failed
    {
        printf("Error: Cannot create file %s\n", decInfo->output_fname);  // Print error message
        return d_failure;                                     // Return failure
    }

    if (extn_size == strlen(decInfo->extn_secret_file))        // Verify decoded extension length matches expected
    {
        return d_success;                                     // Return success if valid
    }

    return d_failure;                                         // Return failure if mismatch
}


// Function to decode the size of the secret file (in bytes) from the stego image
Status1 decode_secret_file_size(DecodeInfo *decInfo)
{
    char image_buffer[32];                                     // Buffer to store 32 bytes (32 bits for size)
    size_t bytesRead;

    bytesRead = fread(image_buffer, 1, 32, decInfo->fptr_stego_image);  // Read 32 bytes from stego image
    if (bytesRead != 32)                                       // Verify if 32 bytes were successfully read
    {
        return d_failure;                                      // Return failure if read error
    }

    decInfo->size_secret_file = decode_size_from_lsb(image_buffer);  // Decode 4-byte integer (file size)
    return d_success;                                          // Return success
}

// Function to decode and write the secret file data into the output file
Status1 decode_secret_file_data(DecodeInfo *decInfo)
{
    char image_buffer[8];                                      // Buffer to hold 8 bytes for one character
    size_t bytesRead;

    for (int i = 0; i < decInfo->size_secret_file; i++)        // Loop for the entire size of the secret file
    {
        bytesRead = fread(image_buffer, 1, 8, decInfo->fptr_stego_image);  // Read 8 bytes for one character
        if (bytesRead != 8)                                    // Check if read failed
        {
            return d_failure;                                  // Return failure if not 8 bytes read
        }

        char ch = decode_byte_from_lsb(image_buffer);          // Decode one character from LSBs

        if (fputc(ch, decInfo->fptr_output_file) == EOF)       // Write decoded character to output file
        {
            return d_failure;                                  // Return failure if write fails
        }
    }

    return d_success;                                          // Return success after decoding all bytes
}

// Main function to coordinate the full decoding process
Status1 do_decoding(DecodeInfo *decInfo)
{
    // Step 1: Open stego image file
    Status1 res = open_files_decode(decInfo);
    if (res == d_failure)
    {
        printf("Error: File does not exist!\n");
        return d_failure;
    } 

    // Step 2: Skip BMP header (first 54 bytes)
    res = skip_bmp_header(decInfo->fptr_stego_image);
    if (res == d_failure)
    {
        printf("Error: skip_bmp_header is failure!\n");
        return d_failure;  
    }
    else
    {
        printf("BMP header skipped successfully!\n");
    }

    // Step 3: Decode and verify magic string
    res = decode_magic_string(decInfo);
    if (res == d_failure)
    {
        printf("Error: Magic string does not match!\n");
        return d_failure;
    }
    else
    {
        printf("Success: Magic string matched!\n");
    }

    // Step 4: Decode size of the secret file extension
    int extn_size = 0;
    res = decode_secret_file_extn_size(decInfo, &extn_size);
    if (res == d_failure)
    {
        printf("Error: decode_secret_file_extn_size failure!\n");
        return d_failure;
    }
    else
    {
        printf("Success: decode_secret_file_extn_size!\n");
    }

    // Step 5: Decode actual secret file extension (e.g., .txt, .c)
    res = decode_secret_file_extn(decInfo, extn_size);
    if (res == d_failure)
    {
        printf("Error: decode_secret_file_extn failure!\n");
        return d_failure;
    }
    else
    {
        printf("Success: decode_secret_file_extn!\n");
    }

    // Step 6: Decode the secret file size
    res = decode_secret_file_size(decInfo);
    if (res == d_failure)
    {
        printf("Error: decode_secret_file_size failure!\n");
        return d_failure;
    }
    else
    {
        printf("Success: decode_secret_file_size!\n");
    }

    // Step 7: Decode the actual secret file data and write it to output
    res = decode_secret_file_data(decInfo);
    if (res == d_failure)
    {
        printf("Error: decode_secret_file_data failure!\n");
        return d_failure;
    }
    else
    {
        printf("Success: decode_secret_file_data!\n");
    }

    // Step 8: Close all opened files
    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_output_file);

    return d_success;                                          // Return overall decoding success
}
