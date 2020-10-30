/*--------------------------------------------------------------------*/
/* checker.c                                                          */
/* Author:                                                            */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "dynarray.h"
#include "checker.h"


/* see checker.h for specification */
boolean Checker_Node_isValid(Node n) {
   Node parent;
   const char* npath;
   const char* ppath;
   const char* rest;
   size_t i;

   /* Sample check: a NULL pointer is not a valid Node */
   if(n == NULL) {
      fprintf(stderr, "Node is a NULL pointer\n");
      return FALSE;
   }

   /* Sample check that the child is not null.
   if(Node_getNumChildren(n) < 0){
      fprintf(stderr, "Node has a negative number of children\n");
      return FALSE;
   } */

   /* Sample check that path exists and is well-defined. */
   npath = Node_getPath(n);
   if(npath == NULL || strlen(npath)<1){
      fprintf(stderr, "Node has invalid path\n");
      return FALSE;
   }
        
   parent = Node_getParent(n);
   if(parent != NULL) {
      npath = Node_getPath(n);

      /* Sample check that parent's path must be prefix of n's path */
      ppath = Node_getPath(parent);
      i = strlen(ppath);
      if(strncmp(npath, ppath, i)) {
         fprintf(stderr, "P's path is not a prefix of C's path\n");
         return FALSE;
      }
      /* Sample check that n's path after parent's path + '/'
         must have no further '/' characters */
      rest = npath + i;
      rest++;
      if(strstr(rest, "/") != NULL) {
         fprintf(stderr, "C's path has grandchild of P's path\n");
         return FALSE;
      }
   }
   else{
      if(strstr(npath, "/") != NULL) {
         fprintf(stderr, "The path has a nonexistent child\n");
         return FALSE;
      }
   }

   return TRUE;
}

/*
   Performs a pre-order traversal of the tree rooted at n.
   Returns FALSE if a broken invariant is found and
   returns TRUE otherwise.

   You may want to change this function's return type or
   parameter list to facilitate constructing your checks.
   If you do, you should update this function comment.
*/
static size_t Checker_treeCheck(Node n, size_t *counter) {
   size_t c;
   char *previousPath;
   if(n != NULL) {

      /* Sample check on each non-root Node: Node must be valid */
      /* If not, pass that failure back up immediately */
      if(!Checker_Node_isValid(n))
         return *counter;


      for(c = 0; c < Node_getNumChildren(n); c++)
      {
         Node child = Node_getChild(n, c);
         if(c == 0){
            previousPath = (char*) Node_getPath(child);
         }
         else{
            if(strcmp((const char*)previousPath,
                      Node_getPath(child)) >= 0){
                  fprintf(stderr, "Children are incorrectly ordered\n");
                  return *counter;
            }
         }
               
         /* if recurring down one subtree results in a failed check
            farther down, passes the failure back up immediately */
         if(!Checker_treeCheck(child, counter))
            return *counter;
         
         if(strcmp(Node_getPath(Node_getParent(child)),
                      Node_getPath(n)) != 0){
               fprintf(stderr, "Child's stored parent is wrong\n");
               return *counter;
         }

         if(child != NULL){
            *counter++;
         }
      }
         
   }
   return *counter;
}

/* see checker.h for specification */
boolean Checker_DT_isValid(boolean isInit, Node root, size_t count) {
   size_t temp;
   size_t rightCount;
   size_t *counter;

   temp = 0;
   counter = &temp;
   /* Sample check on a top-level data structure invariant:
      if the DT is not initialized, its count should be 0. */
   if(!isInit){
      if(count != 0) {
         fprintf(stderr, "Not initialized, but count is not 0\n");
         return FALSE;
      }
      if(root != NULL){
         fprintf(stderr, "Not initialized, but root exists\n");
         return FALSE;
      }
   }
   /* Now checks invariants recursively at each Node from the root. */
   rightCount = Checker_treeCheck(root, counter);
   if(rightCount != count){
      fprintf(stderr, "Miscalculated number of nodes in DT\n");
      return FALSE;
   }
   
   return TRUE;
}
