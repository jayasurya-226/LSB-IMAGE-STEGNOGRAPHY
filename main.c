#include <stdio.h>               // Standard I/O library
#include "encode.h"              // Encoding function declarations
#include "types.h"               // Custom type definitions (Status, OperationType)
#include "types1.h"              // Custom type definitions for decoding (Status1)
#include <string.h>              // String manipulation functions
#include "decode.h"              // Decoding function declarations

void interactive_mode(); // Function prototype

OperationType check_operation_type(char *); // Function prototype to check -e or -d

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;             // Declare encoding information structure

    if (argc == 4 || argc == 5)     // Check correct number of command-line arguments
    {
        OperationType res = check_operation_type(argv[1]); // Determine operation type
        if (res == e_encode)        // If encoding mode selected
        {
            Status res = read_and_validate_encode_args(argv, &encInfo); // Validate input/output files
            if (res == e_success)   // If validation successful
            {
                res = do_encoding(&encInfo); // Perform encoding
                if (res == e_success)       // If encoding succeeds
                    printf("Encoding the secret data successfully!\n");
                else                        // If encoding fails
                {
                    printf("Error: Encoding stop!\n");
                    return e_failure;
                }
            }
            else                         // Validation failed
            {
                printf("Error: Invalid file Extension!\n");
                return e_failure;
            }
        }
        else if (res == e_decode)       // If decoding mode selected
        {
            DecodeInfo decInfo;          // Declare decoding information structure
            Status1 res = read_and_validate_decode_file(argv, &decInfo); // Validate files for decoding
            if (res == d_success)       // If validation successful
            {
                res = do_decoding(&decInfo); // Perform decoding
                if (res == d_success)       // If decoding succeeds
                    printf("Decoding successful!\n");
                else                        // If decoding fails
                {
                    printf("Error: decode_secret_file_data failure!\n");
                    return e_failure;
                }
            }
            else                          // Validation failed
            {
                printf("Error: Invalid file Extention!\n");
                return e_failure;
            }
        }
        else                             // Invalid operation type
        {
            printf("Error: Invalid mode of operation entered:\n");
            return e_failure;
        }
    }
    else                                 // Incorrect number of arguments
    {
         printf(" Error: Incorrect number of arguments.\n");
        printf("Usage: %s <-e/-d> <source_image> <secret_file/output_file>\n", argv[0]);
        interactive_mode(); // calling func
    }
}

/* Function to check operation type: -e for encode, -d for decode */
OperationType check_operation_type(char *symbol)
{
    if (strcmp("-e", symbol) == 0)     // If "-e" entered
        return e_encode;               // Return encoding operation
    else if (strcmp("-d", symbol) == 0) // If "-d" entered
        return e_decode;               // Return decoding operation
    else                               // If neither
        return e_unsupported;          // Return unsupported operation
}
// -------------------- Interactive function --------------------
void interactive_mode()
{
    char mode[3], src[100], dest[100];

    printf("🔁 Please re-enter details:\n");
    printf("Enter mode (-e / -d): ");
    scanf("%2s", mode);

    printf("Enter source image file name: ");
    scanf("%s", src);

    if (strcmp(mode, "-e") == 0)
    {
        printf("Enter output file name: ");
        scanf("%s", dest);

        // Create EncodeInfo from inputs
        EncodeInfo encInfo;
        // initialize encInfo with src and dest
        // Then call your existing encoding function
        // do_encoding(&encInfo);
        printf("✅ Encoding: %s -> %s\n", src, dest);
    }
    else if (strcmp(mode, "-d") == 0)
    {
        printf("Enter secret/output file name (optional, press Enter to skip): ");
        scanf("%s", dest);

        DecodeInfo decInfo;
        // initialize decInfo with src and dest
        // Then call your existing decoding function
        // do_decoding(&decInfo);
        printf("✅ Decoding: %s -> %s\n", src, dest);
    }
    else
    {
        printf(" Error: Invalid mode entered!\n");
        interactive_mode(); // retry recursively
    }
}
