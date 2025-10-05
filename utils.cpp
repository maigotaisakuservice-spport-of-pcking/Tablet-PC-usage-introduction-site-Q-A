#include "utils.h"

// Custom lightweight strcpy
void tiny_strcpy(char* dest, const char* src) {
    while ((*dest++ = *src++));
}

// Custom lightweight sprintf for "world_%d.world"
void tiny_sprintf_world(char* dest, int world_num) {
    const char* prefix = "world_";
    const char* suffix = ".world";

    while ((*dest++ = *prefix++));
    dest--; // Go back to the null terminator

    char num_buf[4]; // Max 3 digits for world number
    int i = 0;
    if (world_num == 0) {
        num_buf[i++] = '0';
    } else {
        while (world_num > 0) {
            num_buf[i++] = (world_num % 10) + '0';
            world_num /= 10;
        }
    }
    while (i > 0) {
        *dest++ = num_buf[--i];
    }

    while ((*dest++ = *suffix++));
}

// Lightweight integer-to-string conversion
void tiny_itoa(int value, char* str) {
    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    int i = 0;
    bool is_negative = value < 0;
    if (is_negative) {
        value = -value;
    }
    // Handle INT_MIN
    unsigned int u_value = value;

    while (u_value != 0) {
        int rem = u_value % 10;
        str[i++] = rem + '0';
        u_value = u_value / 10;
    }
    if (is_negative) {
        str[i++] = '-';
    }
    str[i] = '\0';
    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}