/*************************************************************************************
  Copyright (c) 2021 SingleStore, Inc.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not see <http://www.gnu.org/licenses>
  or write to the Free Software Foundation, Inc.,
  51 Franklin St., Fifth Floor, Boston, MA 02110, USA
*************************************************************************************/

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

struct ASTNode *newLeaf(const char* const lexeme);
struct ASTNode *newNode(int childrenCount, va_list children);
struct ASTNode *copyTree(struct ASTNode *root);
void appendToString(MADB_DynString *res, struct ASTNode *x);
void freeTree(struct ASTNode *x);

#endif //MARIADB_CONNECTOR_C_AST_H
