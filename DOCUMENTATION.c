/*Steganography using LSB (Least Significant Bit) in BMP Images

Name: B. JAYASURYA
Date: 11/10/2025

Project: Image Steganography by LSB  Encode/Decode

Description:

This project allows hiding secret data inside BMP images using the LSB technique.
It supports encoding secret files (.txt, .c, .csv) into BMP images and decoding them back to retrieve the original secret files.
This project implements a steganography system that enables securely hiding secret data within BMP images using the Least Significant Bit (LSB) technique.
The system provides two primary functionalities: encoding and decoding. During encoding, a secret file — which can be a
text file (.txt), a C source file (.c), or a CSV file (.csv)  is embedded into the pixel data of a BMP image.
The LSB technique modifies only the least significant bit of each pixel’s color component, allowing the secret data to be
stored without visibly altering the original image. To ensure proper decoding, the program also embeds metadata,
including the secret file’s extension, file size, and a predefined magic string, which helps in identifying valid stego images.

During decoding, the system reads the stego image, verifies the magic string, and extracts the hidden data by retrieving the least significant bits
from the pixel data. The secret file is then reconstructed byte by byte and saved using its original file extension.
The program ensures that the extracted file is identical to the original secret file, maintaining both accuracy and integrity.
Error handling is implemented at each stage, including file validation, capacity checks, and header preservation, to prevent data corruption
or incomplete encoding/decoding.

Overall, this project demonstrates the practical application of LSB-based steganography for secure data storage and covert communication.
It highlights the importance of metadata management, file validation, and efficient use of image capacity while providing a reliable mechanism
for hiding and retrieving secret information. The system is designed to be user-friendly and robust, making it a valuable tool for educational
and experimental purposes in digital security and data privacy.
1. Project Overview

Steganography is the art of hiding information in a cover medium (here, BMP images) such that the existence of the secret information is
not detectable. This project uses the LSB technique, where the least significant bit of each byte of the image is modified to store secret data.

Features:

Encode secret file into a BMP image.

Decode secret data from a stego BMP image.

Supports multiple secret file extensions: .txt, .c, .csv.

Validates file sizes and extensions before encoding/decoding.

Stores magic string #* to identify stego images.


2. Project Files

File Name	Purpose
encode.h	Function declarations for encoding
encode.c	Encoding functions implementation
decode.h	Function declarations for decoding
decode.c	Decoding functions implementation
types.h	Custom types for encoding (Status, OperationType, uint)
types1.h	Custom types for decoding (Status1, uint)
common.h	Common definitions (e.g., MAGIC_STRING)
main.c	Main driver to handle command-line arguments and call encode/decode

3. Header File Documentation

3.1 types.h

typedef unsigned int uint;           // Alias for unsigned int
typedef enum { e_failure, e_success } Status;       // Return status for encode functions
typedef enum { e_encode, e_decode, e_unsupported } OperationType;  // Mode of operation

3.2 types1.h

typedef unsigned int uint;           // Alias for unsigned int
typedef enum { d_failure, d_success } Status1;      // Return status for decode functions

3.3 common.h

#define MAGIC_STRING "#*"           // Magic string to identify stego images

3.4 encode.h

Contains all encoding function declarations:

get_image_size_for_bmp()

get_file_size()

read_and_validate_encode_args()

open_files()

check_capacity()

copy_bmp_header()

encode_byte_to_lsb()

encode_size_to_lsb()

encode_magic_string()

encode_secret_file_extn_size()

encode_secret_file_extn()

encode_secret_file_size()

encode_secret_file_data()

copy_remaining_img_data()

do_encoding()


3.5 decode.h

Contains all decoding function declarations:

read_and_validate_decode_file()

do_decoding()

open_files_decode()

skip_bmp_header()

decode_byte_from_lsb()

decode_size_from_lsb()

decode_magic_string()

decode_secret_file_extn_size()

decode_secret_file_extn()

decode_secret_file_size()

decode_secret_file_data()


4. Main Program Flow (main.c)

4.1 Steps

Check the number of command-line arguments (4 or 5).

Call check_operation_type(argv[1]) to determine encode or decode.

Encode mode (-e):

Call read_and_validate_encode_args() to validate source image, secret file, and optional stego image filename.

Call do_encoding() to hide secret data in the BMP image.

Print success or error messages.

Decode mode (-d):

Call read_and_validate_decode_file() to validate stego image and output file.

Call do_decoding() to extract secret data from the stego image.

Print success or error messages.

Print usage instructions if arguments are invalid.

5. Encoding Steps


Open files: Source image, secret file, and stego image.

Check capacity: Ensure BMP can store all secret data + header + magic string.

Copy BMP header: First 54 bytes remain unchanged.

Encode magic string: Store #* in LSBs to identify stego images.

Encode secret file extension and size: Store .txt/.c/.csv length and file size in LSBs.

Encode secret data: Each byte of secret file is stored using 8 image bytes.

Copy remaining image bytes: Preserve rest of the image without modification.

Close all files.

6. Decoding Steps


Open stego image and prepare output file.

Skip BMP header: First 54 bytes are skipped.

Check magic string: Ensure #* exists to confirm stego image.

Read secret file extension size and extension.

Read secret file size.

Decode secret data: Extract each byte from 8 image bytes.

Write secret data to output file.

Close all files.

7. LSB Encoding & Decoding


Encoding a byte: Each bit of the secret byte replaces the least significant bit of a byte from image.

Decoding a byte: Extract the least significant bit of 8 consecutive image bytes to reconstruct the secret byte.

8. Error Handling


Invalid file extensions: .bmp for images, .txt/.c/.csv for secret files.

Insufficient image capacity to hold secret data.

File open errors.

Magic string mismatch during decoding.

9. Encoding

./stego -e source_image.bmp secret_file.txt [stego_image.bmp]

10. Decoding

./stego -d stego_image.bmp output_file.txt

11. Example:

a.   ./stego -e cover.bmp secret.txt secret_stego.bmp   ->Encode
b.   ./stego -d secret_stego.bmp recovered_secret.txt   ->Decode
*/


