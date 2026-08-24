#ifndef AST_H
#define AST_H

#include "types/token.h"

/*
* AST node type. Nodes are simply wrappers around a token_t pointer which points
* to the tokens array in memory.
*/
typedef struct node_t{
    const token_t *token;
    struct node_t *left;
    struct node_t *right;
} node_t;


/* AST type */
typedef struct {
    node_t *root;
} ast_t;


/*
* Recursively frees an AST with root `ast->root` and also `ast` itself.
*/
void fully_free_ast(ast_t *ast);


/*
* Recursively frees a subtree rooted at `node`. Note that the token_t pointed
* at by `node->token` is NOT freed.
*/
void free_subtree(node_t *node);


/*
* Initializes a node_t struct with parameters. Returns a pointer to a heap-allocated
* node.
*
* If memory for the node cannot be malloc'd, NULL is returned. 
*/
node_t *initialize_node(const token_t *token, node_t *left, node_t *right);


#endif
