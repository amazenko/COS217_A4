/*--------------------------------------------------------------------*/
/* ft.h                                                               */
/* Author: Alex Baroody & Austen Mazenko                              */
/*--------------------------------------------------------------------*/

#ifndef HANDLER_INCLUDED
#define HANDLER_INCLUDED

#include <stddef.h>
#include "a4def.h"
#include "node.h"

/* Starting at the parameter curr, traverses as far down the file tree
   as possible while still matching the path parameter. Returns a
   pointer to the farthest matching Node down that path, or NULL if
   there is no node in curr's hierarchy that matches a prefix of path */
Node HANDLER_traversePathFrom(char* path, Node curr);

/* Given a prospective parent and child Node,
   adds child to parent's children list, if possible.
   If not possible, destroys the hierarchy rooted at child
   and returns PARENT_CHILD_ERROR, else, returns SUCCESS. */
int HANDLER_linkParentToChild(Node parent, Node child);

/* Alternate version of strlen that uses pAcc as an in-out parameter
   to accumulate a string length, rather than returning the length of
   str, and also always adds one more in addition to str's length. */
void HANDLER_strlenAccumulate(char* str, size_t* pAcc);

/* Alternate version of strcat that inverts the typical argument
   order, appending str onto acc, and also always adds a newline at
   the end of the concatenated string. */
void HANDLER_strcatAccumulate(char* str, char* acc);

#endif