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
#include "handler.h"

/*--------------------------------------------------------------------*/

/* A File Tree is an AO with three state variables: */
/* a flag for if it is in an initialized state (TRUE) or not (FALSE) */
static boolean isInitialized;
/* a pointer to the root Node in the hierarchy */
static Node root;
/* a counter of the number of Nodes in the hierarchy */
static size_t count;


/* Inserts a new path into the tree rooted at parent, with leaf being
   a node with path path, contents contents, length length, and type
   as its value for isFile.
   If the root of the data structure is NULL, then inserts this path's
   first node as the root.
   If the parent is NULL but there exists a root in the tree, return
   CONFLICTING_PATH.
   If the given path exists, return ALREADY_IN_TREE.
   If parent is a file, return NOT_A_DIRECTORY.
   If there's an allocation error in creating any of the new nodes or
   their fields, return MEMORY_ERROR.
   If there is an error linking any of the new nodes, return
   PARENT_CHILD_ERROR.
   Else, return SUCCESS. */
static int FT_insertRestOfPath(char* path, Node parent, boolean type,
                               void *contents, size_t length) {
   /* The node of which the added Node(s) will be a child. */
   Node curr = parent;
   /* The child to be added to curr, and the Node a the path's head. */
   Node firstNew = NULL;
   Node new;
   char* copyPath;
   char* restPath = path;
   char* dirToken;
   char* dirNextToken;
   int result;
   size_t newCount = 0;

   assert(path != NULL);

   if(curr == NULL){
      if(root != NULL) {
      /* If there is a root, but the parent is NULL,
         then it is a conflicting path error.
         NOTE: ft.h stipulates we should return NO_SUCH_PATH
         instead if this happens for a file, but the checker
         suggests the more generally valid approach we use here. */
      return CONFLICTING_PATH;
      }
   }
   else if(!strcmp(path, Node_getPath(curr)))
      return ALREADY_IN_TREE;
   else if (Node_isFile(curr))
      return NOT_A_DIRECTORY;
   else {
      /* If there are no path issues, restPath denotes the portion of
         the path which remains to be added to the data structure. */
      restPath += (strlen(Node_getPath(curr)) + 1);
   }

   /* Copy restPath into copyPath, which is then tokenized in order to
      determine the names of the nodes to be inserted. */
   copyPath = malloc(strlen(restPath)+1);
   if(copyPath == NULL)
      return MEMORY_ERROR;
   strcpy(copyPath, restPath);
   /* Stores the local path name of the next node to be added. */
   dirToken = strtok(copyPath, "/");
   /* Stores the local path name of the node after the node to add. */
   dirNextToken = strtok(NULL, "/");

   /* While the node to add has a "child," meaning it's not a leaf,
      iteratively add it to the tree. */
   while(dirNextToken != NULL) {
      /* If it's not a leaf, it cannot be a file so make new dir. */
      new = Node_createDir(dirToken, curr);
      newCount++;

      if(firstNew == NULL)
         firstNew = new;
      else {
         /* For nodes other than the first, attempt to link them in
            the order dictated by the path. */
         result = HANDLER_linkParentToChild(curr, new);
         if(result != SUCCESS) {
            (void) Node_destroy(new);
            (void) Node_destroy(firstNew);
            free(copyPath);
            return result;
         }
      }

      /* If there is no non-null node added, there was a MEMORY_ERROR */
      if(new == NULL) {
         (void) Node_destroy(firstNew);
         free(copyPath);
         return MEMORY_ERROR;
      }

      /* Parent becomes the node just added, and the two tokens
         iterate one step down the path to determine which nodes will
         be added next. */
      curr = new;
      dirToken = dirNextToken;
      dirNextToken = strtok(NULL, "/");
   }

   /* If the child is NULL but there is another node to add, we enter
      this block and append the leaf to the data structure. Internally,
      it's analogous to the previous for loop.*/
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
         result = HANDLER_linkParentToChild(curr, new);
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

   /* If the tree was initially empty, let this inserted path be the
            entire data structure. */
   if(parent == NULL) {
      root = firstNew;
      count = newCount;
      return SUCCESS;
   }
   else {
      /* Link the added path to the data structure. */
      result = HANDLER_linkParentToChild(parent, firstNew);
      if(result == SUCCESS)
         count += newCount;
      else
         (void) Node_destroy(firstNew);
      return result;
   }
}

/* Removes the directory hierarchy rooted at path starting from Node
   curr. If curr is the data structure's root, root becomes NULL.
   Returns NO_SUCH_PATH if curr is not the Node for path,
   otherwise SUCCESS. */
static int FT_rmPathAt(char* path, Node curr) {
   Node parent;

   assert(path != NULL);
   assert(curr != NULL);

   parent = Node_getParent(curr);

   if(!strcmp(path, Node_getPath(curr))) {
      if(parent == NULL){
         root = NULL;
      }
      else
         Node_unlinkChild(parent, curr);

      if(curr != NULL) {
         count -= Node_destroy(curr);
      }

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
   curr = HANDLER_traversePathFrom(path, root);
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

   curr = HANDLER_traversePathFrom(path, root);

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
      result = INITIALIZATION_ERROR;

   curr = HANDLER_traversePathFrom(path, root);
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
   curr = HANDLER_traversePathFrom(path, root);
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

   curr = HANDLER_traversePathFrom(path, root);

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

   curr = HANDLER_traversePathFrom(path, root);
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

   curr = HANDLER_traversePathFrom(path, root);

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

   curr = HANDLER_traversePathFrom(path, root);

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
   assert(type != NULL);
   assert(length != NULL);

   if(!isInitialized)
      result = INITIALIZATION_ERROR ;

   curr = HANDLER_traversePathFrom(path, root);

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

   DynArray_map(nodes, (void (*)(void *, void*)) HANDLER_strlenAccumulate ,
                (void*) &totalStrlen);

   result = malloc(totalStrlen);
   if(result == NULL) {
      DynArray_free(nodes);
      assert(Checker_FT_isValid(isInitialized, root, count));
      return NULL;
   }

   *result = '\0';

   DynArray_map(nodes, (void (*)(void *, void*)) HANDLER_strcatAccumulate,
                (void *) result);

   DynArray_free(nodes);
   assert(Checker_FT_isValid(isInitialized, root, count));
   return result;
}
