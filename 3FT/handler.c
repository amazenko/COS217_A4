/*--------------------------------------------------------------------*/
/* ft.h                                                               */
/* Author: Alex Baroody and Austen Mazenko                            */
/*--------------------------------------------------------------------*/

#include <stddef.h>
#include "a4def.h"
#include "node.h"

/* Starting at the parameter curr, traverses as far down the file tree
   as possible while still matching the path parameter. Returns a
   pointer to the farthest matching Node down that path, or NULL if
   there is no node in curr's hierarchy that matches a prefix of path */
Node HANDLER_traversePathFrom(char* path, Node curr) {
   Node found;
   size_t i;
   char* currPath;

   if(curr == NULL)
      return NULL;

   currPath = (char*)Node_getPath(curr);
   if(!strcmp(path, currPath))
      return curr;

   else if(!strncmp(path, currPath,
                    strlen(currPath))) {
      if(Node_isFile(curr))
         return curr;
      for(i = 0; i < Node_getNumChildren(curr); i++) {
         found = FT_traversePathFrom(path, Node_getChild(curr,i));
         if(found != NULL)
            return found;
      }
      return curr;
   }
   return NULL;
}
   

/* Given a prospective parent and child Node,
   adds child to parent's children list, if possible.
   If not possible, destroys the hierarchy rooted at child
   and returns PARENT_CHILD_ERROR, else, returns SUCCESS. */
int HANDLER_linkParentToChild(Node parent, Node child) {
   assert(parent != NULL);

   if(Node_linkChild(parent, child) != SUCCESS) {
      (void) Node_destroy(child);
      return PARENT_CHILD_ERROR;
   }

   return SUCCESS;
}

/* Alternate version of strlen that uses pAcc as an in-out parameter
   to accumulate a string length, rather than returning the length of
   str, and also always adds one more in addition to str's length. */
void HANDLER_strlenAccumulate(char* str, size_t* pAcc) {
   assert(pAcc != NULL);

   if(str != NULL)
      *pAcc += (strlen(str) + 1);
}

/* Alternate version of strcat that inverts the typical argument
   order, appending str onto acc, and also always adds a newline at
   the end of the concatenated string. */
void HANDLER_strcatAccumulate(char* str, char* acc) {
   assert(acc != NULL);

   if(str != NULL)
      strcat(acc, str); strcat(acc, "\n");
}