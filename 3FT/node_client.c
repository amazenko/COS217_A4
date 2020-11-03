/*--------------------------------------------------------------------*/
/* ft_client.c                                                        */
/* Author: Christopher Moretti                                        */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "node.h"

/* Tests the node implementation with an assortment of checks.
   Prints the status of the data structure along the way to stderr.
   Returns 0. */
int main(void) {
   Node nodeA;
   Node nodeB;
   Node nodeC;
   Node nodeD;
   char *temp;

   assert((nodeA = Node_createDir("a", NULL)) != NULL);
   assert(Node_getChild(nodeA, 0) == NULL);
   assert(Node_getNumChildren(nodeA) == 0);
   assert(Node_getParent(nodeA) == NULL);
   assert(!Node_isFile(nodeA));
   assert(strcmp(Node_getPath(nodeA), "a") == 0);
   assert((temp = Node_toString(nodeA)) != NULL);
   fprintf(stderr, "%s\n", temp);
   free(temp);
   assert(Node_destroy(nodeA) == 1);

   assert((nodeA = Node_createFile("a", NULL)) != NULL);
   assert(Node_getChild(nodeA, 0) == NULL);
   assert(Node_getNumChildren(nodeA) == 0);
   assert(Node_getParent(nodeA) == NULL);
   assert(Node_isFile(nodeA));
   assert(strcmp(Node_getPath(nodeA), "a") == 0);
   assert((temp = Node_toString(nodeA)) != NULL);
   fprintf(stderr, "%s\n", temp);
   free(temp);
   assert(Node_destroy(nodeA) == 1);

   assert((nodeA = Node_createDir("a", NULL)) != NULL);
   assert((nodeB = Node_createFile("a/b", NULL)) != NULL);
   assert(Node_linkChild(nodeA, nodeB) == SUCCESS);
   assert(Node_compare(nodeA, nodeB) != 0);
   assert(Node_compare(Node_getChild(nodeA, 0), nodeB) == 0);
   assert(Node_getNumChildren(nodeA) == 1);
   assert(Node_compare(Node_getParent(nodeB), nodeA) == 0);
   assert((temp = Node_toString(nodeA)) != NULL);
   fprintf(stderr, "%s\n", temp);
   free(temp);
   assert((temp = Node_toString(nodeB)) != NULL);
   fprintf(stderr, "%s\n", temp);
   free(temp);
   assert(Node_unlinkChild(nodeA, nodeB) == SUCCESS);
   assert(Node_getNumChildren(nodeA) == 0);
   assert(Node_getChild(nodeA, 0) == NULL);
   assert(Node_getParent(nodeB) == NULL);
   assert(Node_destroy(nodeB) == 1);
   assert(Node_destroy(nodeA) == 1);

   assert((nodeA = Node_createDir("a", NULL)) != NULL);
   assert((nodeB = Node_createDir("a/b", NULL)) != NULL);
   assert((nodeC = Node_createFile("a/c", NULL)) != NULL);
   assert((nodeD = Node_createFile("a/b/d", NULL)) != NULL);
   assert(Node_linkChild(nodeA, nodeB) == SUCCESS);
   assert(Node_linkChild(nodeA, nodeC) == SUCCESS);
   assert(Node_linkChild(nodeB, nodeD) == SUCCESS);
   assert(Node_compare(nodeA, nodeB) != 0);
   assert(Node_compare(nodeA, nodeC) != 0);
   assert(Node_compare(nodeA, nodeD) != 0);
   assert(Node_compare(nodeB, nodeC) != 0);
   assert(Node_compare(nodeB, nodeD) != 0);
   assert(Node_compare(nodeC, nodeD) != 0);
   assert(Node_compare(Node_getChild(nodeA, 0), nodeB) == 0);
   assert(Node_compare(Node_getChild(nodeA, 1), nodeC) == 0);
   assert(Node_compare(Node_getChild(nodeB, 0), nodeD) == 0);
   assert(Node_getNumChildren(nodeA) == 2);
   assert(Node_getNumChildren(nodeB) == 1);
   assert(Node_compare(Node_getParent(nodeB), nodeA) == 0);
   assert(Node_compare(Node_getParent(nodeC), nodeA) == 0);
   assert(Node_compare(Node_getParent(nodeD), nodeB) == 0);
   assert((temp = Node_toString(nodeA)) != NULL);
   fprintf(stderr, "%s\n", temp);
   free(temp);
   assert((temp = Node_toString(nodeB)) != NULL);
   fprintf(stderr, "%s\n", temp);
   free(temp);
   assert((temp = Node_toString(nodeC)) != NULL);
   fprintf(stderr, "%s\n", temp);
   free(temp);
   assert((temp = Node_toString(nodeD)) != NULL);
   fprintf(stderr, "%s\n", temp);
   free(temp);
   assert(Node_unlinkChild(nodeB, nodeD) == SUCCESS);
   assert(Node_unlinkChild(nodeA, nodeB) == SUCCESS);
   assert(Node_unlinkChild(nodeA, nodeC) == SUCCESS);
   assert(Node_getNumChildren(nodeA) == 0);
   assert(Node_getNumChildren(nodeB) == 0);
   assert(Node_getChild(nodeA, 0) == NULL);
   assert(Node_getChild(nodeA, 1) == NULL);
   assert(Node_getChild(nodeB, 0) == NULL);
   assert(Node_getParent(nodeB) == NULL);
   assert(Node_getParent(nodeC) == NULL);
   assert(Node_getParent(nodeD) == NULL);
   assert(Node_destroy(nodeD) == 1);
   assert(Node_destroy(nodeC) == 1);
   assert(Node_destroy(nodeB) == 1);
   assert(Node_destroy(nodeA) == 1);

   return 0;
}
