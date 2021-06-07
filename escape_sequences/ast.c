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

#include "ma_global.h"
#include "ast.h"
#include <stdarg.h>
#include <string.h>

struct ASTNode *newLeaf(const char* const lexeme)
{
  struct ASTNode* x = malloc( sizeof( struct ASTNode ));
  if (x == NULL)
  {
    return NULL;
  }
  x->lexeme = strdup(lexeme);
  if (x->lexeme == NULL)
  {
    free(x);
    return NULL;
  }
  x->totalLength = strlen(lexeme);
  x->childrenCount = 0;
  return x;
}

struct ASTNode *newNode(int childrenCount, va_list children)
{
  struct ASTNode* x = malloc(sizeof(struct ASTNode));
  if (x == NULL)
  {
    return NULL;
  }

  x->childrenCount=childrenCount;
  x->children = malloc(childrenCount * sizeof(struct ASTNode*));
  if (x->children == NULL)
  {
    free(x);
    return NULL;
  }
  x->totalLength = 0;

  int i;
  for (i = 0; i < childrenCount; i++)
  {
    x->children[i] = va_arg(children, struct ASTNode*);
    x->totalLength += x->children[i]->totalLength;
  }

  return x;
}

void appendToString(MADB_DynString *res, struct ASTNode *x)
{
  if (x->childrenCount == 0)
  {
    MADB_DynstrAppend(res, x->lexeme);
  } else
  {
    int i;
    for (i = 0; i < x->childrenCount; i++)
    {
      appendToString(res, x->children[i]);
    }
  }
}

struct ASTNode *copyTree(struct ASTNode *root)
{
  struct ASTNode* x = malloc(sizeof(struct ASTNode));
  if (x == NULL)
  {
    return NULL;
  }

  if (root->childrenCount != 0)
  {
    x->childrenCount=root->childrenCount;
    x->children = malloc(root->childrenCount * sizeof(struct ASTNode*));
    if (x->children == NULL)
    {
      free(x);
      return NULL;
    }
    x->totalLength = root->totalLength;

    int i;
    for (i = 0; i < root->childrenCount; i++)
    {
      x->children[i] = copyTree(root->children[i]);
      if (x->children[i] == NULL)
      {
        int j;
        for (j = 0; j < i; j++)
        {
          freeTree(x->children[j]);
        }
        free(x->children);
        free(x);
        return NULL;
      }
    }
  } else
  {
    x->childrenCount = 0;
    x->totalLength = root->totalLength;
    x->lexeme = strdup(root->lexeme);
    if (x->lexeme == NULL)
    {
      free(x);
      return NULL;
    }
  }

  return x;
}

void freeTree(struct ASTNode *x)
{
  if (x->childrenCount == 0)
  {
    free(x->lexeme);
  } else
  {
    int i;
    for (i = 0; i < x->childrenCount; i++)
    {
      freeTree(x->children[i]);
    }
    free(x->children);
  }

  free(x);
}
