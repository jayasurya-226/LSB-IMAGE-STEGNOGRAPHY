#ifndef TYPES_H
#define TYPES_H

/* Define an unsigned int type alias */
typedef unsigned int uint;                // 'uint' can be used instead of 'unsigned int'

/* Status enum for function return types */
typedef enum
{
    e_failure,                             // Represents a failure in a function
    e_success                              // Represents success in a function
} Status;

/* OperationType enum for mode of operation */
typedef enum
{
    e_encode,                              // Encoding operation
    e_decode,                              // Decoding operation
    e_unsupported                          // Unsupported or invalid operation
} OperationType;

#endif
