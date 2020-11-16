#ifndef _MA_ESCAPE_SEQUENCES_AST_H
#define _MA_ESCAPE_SEQUENCES_AST_H

#include "ma_legacy_helpers.h"

struct ASTNode
{
  struct ASTNode **children;
  int childrenCount;
  size_t totalLength;
  char *lexeme;
};

struct ASTNode *newLeaf(char *lexeme);
struct ASTNode *newNode(int childrenCount, va_list children);
struct ASTNode *copyTree(struct ASTNode *root);
void appendToString(MADB_DynString *res, struct ASTNode *x);
void freeTree(struct ASTNode *x);

#endif //MARIADB_CONNECTOR_C_AST_H
