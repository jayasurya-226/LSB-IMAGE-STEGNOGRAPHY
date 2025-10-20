#ifndef TYPES1_H
#define TYPES1_H

/* Define an unsigned int type alias */
typedef unsigned int uint;               // 'uint' can be used instead of 'unsigned int'

/* Status1 enum for function return types in decoding */
typedef enum
{
    d_failure,                           // Represents a failure in a decoding function
    d_success                            // Represents success in a decoding function
} Status1;

#endif
