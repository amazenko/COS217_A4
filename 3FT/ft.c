/*--------------------------------------------------------------------*/
/* ft.c                                                               */
/* Author(s): Austen Mazenko and Alex Baroody                         */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "dynarray.h"
#include "ft.h"
#include "node.h"
#include "checker.h"

/*--------------------------------------------------------------------*/

/* A File Tree is an AO with three state variables: */
/* a flag for if it is in an initialized state (TRUE) or not (FALSE) */
static boolean isInitialized;
/* a pointer to the root Node in the hierarchy */
static Node root;
/* a counter of the number of Nodes in the hierarchy */
static size_t count;

/* Starting at the parameter curr, traverses as far down the file tree
   as possible while still matching the path parameter. Returns a
   pointer to the farthest matching Node down that path, or NULL if
   there is no node in curr's hierarchy that matches a prefix of path */
static Node FT_traversePathFrom(char* path, Node curr) {
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
   

/* Returns the farthest Node (directory or file) reachable from the root
   following a given path, or NULL if there is no Node in the tree
   which matches a prefix of the path. */
static Node FT_traversePath(char* path) {
   Node curr;
   assert(path != NULL);
   curr = root;
   return FT_traversePathFrom(path, curr);
}

/* Given a prospective parent and child Node,
   adds child to parent's children list, if possible.
   If not possible, destroys the hierarchy rooted at child
   and returns PARENT_CHILD_ERROR, else, returns SUCCESS. */
static int FT_linkParentToChild(Node parent, Node child) {
   assert(parent != NULL);

   if(Node_linkChild(parent, child) != SUCCESS) {
      (void) Node_destroy(child);
      return PARENT_CHILD_ERROR;
   }

   return SUCCESS;
}

/* ISNERT COMMENT - SHOULD ALREADY IN TREE WORK IF ITS FILE IN TREE? */
static int FT_insertRestOfPath(char* path, Node parent, boolean type,
                               void *contents, size_t length) {
   Node curr = parent;
   Node firstNew = NULL;
   Node new;
   char* copyPath;
   char* restPath = path;
   char* dirToken;
   char* dirNextToken;
   int result;
   size_t newCount = 0;

   assert(path != NULL);

   if(curr == NULL) {
      if(type){
         /* return NO_SUCH_PATH;*/
         return CONFLICTING_PATH;
      }
      else if(root != NULL) {
         return CONFLICTING_PATH;
         }
   }
   else if(!strcmp(path, Node_getPath(curr)))
      return ALREADY_IN_TREE;
   else if (Node_isFile(curr))
      return NOT_A_DIRECTORY;
   else {
      restPath += (strlen(Node_getPath(curr)) + 1);
   }

   copyPath = malloc(strlen(restPath)+1);
   if(copyPath == NULL)
      return MEMORY_ERROR;
   strcpy(copyPath, restPath);
   dirToken = strtok(copyPath, "/");
   dirNextToken = strtok(NULL, "/");
   
   while(dirNextToken != NULL) {
      new = Node_createDir(dirToken, curr);
      newCount++;

      if(firstNew == NULL)
         firstNew = new;
      else {
         result = FT_linkParentToChild(curr, new);
         if(result != SUCCESS) {
            (void) Node_destroy(new);
            (void) Node_destroy(firstNew);
            free(copyPath);
            return result;
         }
      }

      if(new == NULL) {
         (void) Node_destroy(firstNew);
         free(copyPath);
         return MEMORY_ERROR;
      }

      curr = new;
      dirToken = dirNextToken;
      dirNextToken = strtok(NULL, "/");
   }

   if(dirToken != NULL) {
      if(type) {
         new = Node_createFile(dirToken, curr);
         Node_setContents(new, contents, length);
      }
      else
         new = Node_createDir(dirToken, curr);
      
      newCount++;

      if(firstNew == NULL)
         firstNew = new;
      else {
         result = FT_linkParentToChild(curr, new);
         if(result != SUCCESS) {
            (void) Node_destroy(new);
            (void) Node_destroy(firstNew);
            free(copyPath);
            return result;
         }
      }

      if(new == NULL) {
         (void) Node_destroy(firstNew);
         free(copyPath);
         return MEMORY_ERROR;
      }
   }


   free(copyPath);

   if(parent == NULL) {
      root = firstNew;
      count = newCount;
      return SUCCESS;
   }
   else {
      result = FT_linkParentToChild(parent, firstNew);
      if(result == SUCCESS)
         count += newCount;
      else
         (void) Node_destroy(firstNew);
      return result;
   }
}

static void FT_removePathFrom(Node curr) {
   if(curr != NULL) {
      count -= Node_destroy(curr);
   }
}

static int FT_rmPathAt(char* path, Node curr) {
   Node parent;

   assert(path != NULL);
   assert(curr != NULL);

   parent = Node_getParent(curr);

   if(!strcmp(path, Node_getPath(curr))) {
      if(parent == NULL){
         root = NULL;
         count = 0;
         isInitialized = FALSE;
      }
      else
         Node_unlinkChild(parent, curr);

      FT_removePathFrom(curr);

      return SUCCESS;
   }
   else
      return NO_SUCH_PATH;
}

/* see ft.h for specification */
int FT_insertDir(char *path){
   Node curr;
   int result;

   assert(Checker_FT_isValid(isInitialized, root, count));
   assert(path != NULL);

   if(!isInitialized)
      return INITIALIZATION_ERROR;
   curr = FT_traversePath(path);
   result = FT_insertRestOfPath(path, curr, FALSE, NULL, 0);

   assert(Checker_FT_isValid(isInitialized, root, count));
   return result;
}

/* see ft.h for specification */
boolean FT_containsDir(char *path){
   Node curr;
   boolean result;

   assert(Checker_FT_isValid(isInitialized, root, count));
   assert(path != NULL);

   if(!isInitialized)
      return FALSE;

   curr = FT_traversePath(path);

   if(curr == NULL)
      result = FALSE;
   else if(strcmp(path, Node_getPath(curr)))
      result = FALSE;
   else
      result = !Node_isFile(curr);

   assert(Checker_FT_isValid(isInitialized, root, count));
   return result;
   
}

/* see ft.h for specification */
int FT_rmDir(char *path){
   Node curr;
   int result;

   assert(Checker_FT_isValid(isInitialized, root, count));
   assert(path != NULL);

   if(!isInitialized)
      return INITIALIZATION_ERROR;

   curr = FT_traversePath(path);
   if(curr == NULL)
      result = NO_SUCH_PATH;
   else if (Node_isFile(curr))
      result = NOT_A_DIRECTORY;
   else
      result = FT_rmPathAt(path, curr);
   
   assert(Checker_FT_isValid(isInitialized, root, count));
   return result;
}

/* see ft.h for specification */
int FT_insertFile(char *path, void *contents, size_t length){
   Node curr;
   int result;

   assert(Checker_FT_isValid(isInitialized, root, count));
   assert(path != NULL);

   if(!isInitialized)
      return INITIALIZATION_ERROR;
   curr = FT_traversePath(path);
   result = FT_insertRestOfPath(path, curr, TRUE, contents, length);
   assert(Checker_FT_isValid(isInitialized, root, count));
   return result;
}

/* see ft.h for specification */
boolean FT_containsFile(char *path){
   Node curr;
   boolean result;

   assert(Checker_FT_isValid(isInitialized, root, count));
   assert(path != NULL);

   if(!isInitialized)
      result = FALSE;

   curr = FT_traversePath(path);

   if(curr == NULL)
      result = FALSE;
   else if(strcmp(path, Node_getPath(curr)))
      result = FALSE;
   else
      result = Node_isFile(curr);

   assert(Checker_FT_isValid(isInitialized, root, count));
   return result;
}

/* see ft.h for specification */
int FT_rmFile(char *path){
   Node curr;
   int result;

   assert(Checker_FT_isValid(isInitialized, root, count));
   assert(path != NULL);

   if(isInitialized)
      result = INITIALIZATION_ERROR;

   curr = FT_traversePath(path);
   if(curr == NULL)
      result = NO_SUCH_PATH;
   else if (!Node_isFile(curr))
      result = NOT_A_FILE;
   else
      result = FT_rmPathAt(path, curr);
   
   assert(Checker_FT_isValid(isInitialized, root, count));
   return result;
}

/* see ft.h for specification */
void *FT_getFileContents(char *path){
   Node curr;
   void *result;

   assert(Checker_FT_isValid(isInitialized, root, count));
   assert(path != NULL);

   if(!isInitialized)
      result = NULL;

   curr = FT_traversePath(path);

   if(curr == NULL || strcmp(path, Node_getPath(curr))
      || !Node_isFile(curr))
      result = NULL;
   else
      result = Node_getContents(curr);

   assert(Checker_FT_isValid(isInitialized, root, count));
   return result;
}

/* see ft.h for specification */
void *FT_replaceFileContents(char *path, void *newContents,
                             size_t newLength){
   Node curr;
   void *result;

   assert(Checker_FT_isValid(isInitialized, root, count));
   assert(path != NULL);

   if(!isInitialized)
      result = NULL;

   curr = FT_traversePath(path);

   if(curr == NULL || strcmp(path, Node_getPath(curr))
      || !Node_isFile(curr))
      result = NULL;
   else
      result = Node_setContents(curr, newContents, newLength);

   assert(Checker_FT_isValid(isInitialized, root, count));
   return result;
}

/* see ft.h for specification */
int FT_stat(char *path, boolean *type, size_t *length){
   Node curr;
   boolean result;

   assert(Checker_FT_isValid(isInitialized, root, count));
   assert(path != NULL);

   if(!isInitialized)
      result = INITIALIZATION_ERROR ;

   curr = FT_traversePath(path);

   if(curr == NULL)
      result = NO_SUCH_PATH;
   else if(strcmp(path, Node_getPath(curr)))
      result = NO_SUCH_PATH;
   else {
      if(Node_isFile(curr)){
         *type = TRUE;
         *length = Node_getLength(curr);
      }
      else
         *type = FALSE;
      result = SUCCESS;
   }

   assert(Checker_FT_isValid(isInitialized, root, count));
   return result;
}

/* see ft.h for specification */
int FT_init(void){
   int result;
   assert(Checker_FT_isValid(isInitialized, root, count));
   if(isInitialized)
      result = INITIALIZATION_ERROR;
   else {
      isInitialized = TRUE;
      root = NULL;
      count = 0;
      result = SUCCESS;
   }
   assert(Checker_FT_isValid(isInitialized, root, count));
   return result;
}

/* see ft.h for specification */
int FT_destroy(void){
   int result;
   assert(Checker_FT_isValid(isInitialized, root, count));
   
   if(!isInitialized)
      result = INITIALIZATION_ERROR;
   else if(root == NULL)
      result = SUCCESS;
   else {
      FT_rmPathAt((char*)Node_getPath(root), root);
      root = NULL;
      count = 0;
      isInitialized = FALSE;
      result = SUCCESS;
   }

   assert(Checker_FT_isValid(isInitialized, root, count));
   return result;
}

/* Performs a pre-order traversal of the tree rooted at n,
   inserting each payload to DynArray_T d beginning at index i.
   Returns the next unused index in d after the insertion(s). */
static size_t FT_preOrderTraversal(Node n, DynArray_T d, size_t i) {
   size_t c;

   assert(d != NULL);

   if(n != NULL) {
      (void) DynArray_set(d, i, Node_getPath(n));
      i++;
      for(c = 0; c < Node_getNumChildren(n); c++)
         i = FT_preOrderTraversal(Node_getChild(n, c), d, i);
   }

   return i;
}

/* Alternate version of strlen that uses pAcc as an in-out parameter
   to accumulate a string length, rather than returning the length of
   str, and also always adds one more in addition to str's length. */
static void FT_strlenAccumulate(char* str, size_t* pAcc) {
   assert(pAcc != NULL);

   if(str != NULL)
      *pAcc += (strlen(str) + 1);
}

/* Alternate version of strcat that inverts the typical argument
   order, appending str onto acc, and also always adds a newline at
   the end of the concatenated string. */
static void FT_strcatAccumulate(char* str, char* acc) {
   assert(acc != NULL);

   if(str != NULL)
      strcat(acc, str); strcat(acc, "\n");
}

/* see ft.h for specification */
char *FT_toString(){
   DynArray_T nodes;
   size_t totalStrlen = 1;
   char* result = NULL;

   assert(Checker_FT_isValid(isInitialized, root, count));

   if(!isInitialized)
      return NULL;

   nodes = DynArray_new(count);
   (void) FT_preOrderTraversal(root, nodes, 0);

   DynArray_map(nodes, (void (*)(void *, void*)) FT_strlenAccumulate,
                (void*) &totalStrlen);

   result = malloc(totalStrlen);
   if(result == NULL) {
      DynArray_free(nodes);
      assert(Checker_FT_isValid(isInitialized, root, count));
      return NULL;
   }

   *result = '\0';

   DynArray_map(nodes, (void (*)(void *, void*)) FT_strcatAccumulate,
                (void *) result);

   DynArray_free(nodes);
   assert(Checker_FT_isValid(isInitialized, root, count));
   return result;
}












