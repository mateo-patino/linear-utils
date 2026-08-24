#ifndef ERRORPRINTER_H
#define ERRORPRINTER_H

#include <stdbool.h>
#include <stdlib.h>


/*
* Fills the buffer with '\0'. Any message in the buffer is deleted.
*/
void clear_error(void) ;


/*
 * Returns a read-only pointer to the error message buffer.
 */
const char *get_error(void);

/*
* Takes a format string 'fmt' and a variable number of arguments to populate the format
* string. It behaves exactly like fprintf(stderr, fmt, ...) except that the formatted
* string is written to an internal buffer which can be inspected via get_error().
*
* Returns true if the formatted string was written to the buffer and false otherwise.
*/
bool set_error(const char *fmt, ...);


/*
* Returns a boolean indicating whether a message exists in the buffer. Calling has_error
* immediately after clear_error returns false; doing so immediately after alling set_error
* returns true.
* 
*/
bool has_error(void);

#endif
