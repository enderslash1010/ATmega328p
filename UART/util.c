#include "util.h"

// Puts int64_t into string array
char* int64ToStringBuffer(char* buffer, int64_t val)
{
	uint8_t digit = 0;
	char reversed[20]; // 19 is max digits in int64_t, plus 1 for null terminator
	char *ptr = reversed;
	
	int64_t n = val;
	if (n < 0) n *= -1; // Treat n as a positive number
	
	while (n)
	{
		digit = n % 10;
		n /= 10;
		
		*(ptr++) = digit + '0'; // Puts digits in reversed in reverse order
	}
	if (val < 0) *(ptr++) = '-'; // Insert negative sign if needed
	
	int length = ptr - reversed - 1; // Determine length of number in characters
	int i = 0;
	for (; i <= length; i++)
	{
		buffer[i] = *(--ptr); // Puts digits in buffer in forward order
	}
	buffer[i] = '\0'; // Insert null terminator
	return buffer;
}
