#include "ma_global.h"
#include <ma_sys.h>
#include "ma_odbc.h"

#define DEFAULT_CATALOG "def"
#define DEFAULT_TABLE "SUB"
#define STRING_CHARSET 33 // utf8mb3
#define DEFAULT_CHARSET 63 // binary

SQLRETURN constructField(MA_MEM_ROOT *alloc, MYSQL_FIELD* dest, char* fieldName, enum enum_field_types type, unsigned long length)
{
    dest->name_length = dest->org_name_length = strlen(fieldName);
    if (!(dest->name = ma_strdup_root(alloc, fieldName))) { return SQL_ERROR; }
    dest->org_name = dest->name;
    dest->catalog_length = strlen(DEFAULT_CATALOG);
    dest->catalog = DEFAULT_CATALOG;
    dest->table_length = strlen(DEFAULT_TABLE);
    dest->table = DEFAULT_TABLE;

    dest->db_length = dest->org_table_length = dest->def_length = 0;
    dest->db = "";
    dest->org_table = "";
    dest->def = NULL;

    dest->decimals = 0;
    dest->flags = 0;
    dest->type = type;
    switch (type) {
        case MYSQL_TYPE_BLOB:
        case MYSQL_TYPE_LONG_BLOB:
        case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_TINY_BLOB:
            dest->flags |= BLOB_FLAG;
        case MYSQL_TYPE_VARCHAR:
        case MYSQL_TYPE_STRING:
        case MYSQL_TYPE_VAR_STRING:
            dest->charsetnr = STRING_CHARSET;
            length += 1; // Terminal symbol
            break;
        default:
            dest->charsetnr = DEFAULT_CHARSET;
    }

    MARIADB_CHARSET_INFO *fieldCs = mariadb_get_charset_by_nr(dest->charsetnr);
    dest->length = dest->max_length = length * fieldCs->char_maxlen;
    return SQL_SUCCESS;
}

SQLRETURN constructData(MYSQL_DATA *dest, char ***data, unsigned long dataLength, unsigned long rowLength, unsigned long *max_lengths)
{
    dest->rows = dataLength;
    dest->fields = rowLength;
    dest->type = MYSQL_FAKE_RESULT;
    MYSQL_ROWS *cur = dest->data = (MYSQL_ROWS *) ma_alloc_root(&dest->alloc, dataLength * sizeof(MYSQL_ROWS));
    if (!cur) { return SQL_ERROR; }

    int i;
    unsigned long j;
    for (i = 0; i < dataLength; i++)
    {
        if (!(cur->data = (char**)ma_alloc_root(&dest->alloc, sizeof(char*) * (rowLength)))) {
            return SQL_ERROR;
        }
        cur->length = 0;
        for (j = 0; j < rowLength; j++)
        {
            if (data[i][j] != NULL) {
                cur->length += strlen(data[i][j]);
                max_lengths[j] = MAX(max_lengths[j], strlen(data[i][j]));
                if (!(cur->data[j] = ma_strdup_root(&dest->alloc, data[i][j]))) {
                    return SQL_ERROR;
                }
            } else {
                cur->data[j] = NULL;
            }
        }
        if (i < dataLength - 1) {
            cur = cur->next = cur + 1;
        } else {
            cur->next = NULL;
        }
    }
    return SQL_SUCCESS;
}

SQLRETURN MADB_FakeRequest(MADB_Stmt *Stmt, char **fields, enum enum_field_types *fieldTypes, unsigned long fieldsLength,
                       char ***data, unsigned long dataLength)
{
    if (dataLength == 0) {
        return MADB_SetError(&Stmt->Error, MADB_ERR_HY090, "Non-empty data required", 0);
    }
    if (fieldsLength == 0) {
        return MADB_SetError(&Stmt->Error, MADB_ERR_HY090, "Non-empty fields required", 0);
    }
    if (fields == NULL) {
        return MADB_SetError(&Stmt->Error, MADB_ERR_HY009, "Fields value required", 0);
    }
    if (fieldTypes == NULL) {
        return MADB_SetError(&Stmt->Error, MADB_ERR_HY009, "Fields types value required", 0);
    }
    if (data == NULL) {
        return MADB_SetError(&Stmt->Error, MADB_ERR_HY009, "Data value required", 0);
    }

    LOCK_MARIADB(Stmt->Connection);
    MADB_StmtReset(Stmt);
    UNLOCK_MARIADB(Stmt->Connection);
    MA_MEM_ROOT *alloc_root = &(Stmt->stmt->mem_root);
    MA_MEM_ROOT *fields_ma_alloc_root= &((MADB_STMT_EXTENSION *)Stmt->stmt->extension)->fields_ma_alloc_root;
    Stmt->stmt->field_count = fieldsLength;

    // Check if allocation didn't fail
    if (!(Stmt->stmt->fields = (MYSQL_FIELD*) ma_alloc_root(fields_ma_alloc_root, sizeof(MYSQL_FIELD)*fieldsLength)) ||
        !(Stmt->stmt->bind= (MYSQL_BIND *)ma_alloc_root(alloc_root, fieldsLength * sizeof(MYSQL_BIND))))
    {
        ma_free_root(fields_ma_alloc_root, MYF(0));
        ma_free_root(alloc_root, MYF(0));
        char *errMsg = "Failed to allocate memory for fields";
        if(Stmt->stmt->fields) {
            errMsg = "Failed to allocate memory for bind";
        }
        return MADB_SetError(&Stmt->Error, MADB_ERR_HY001, errMsg, 0);
    }
    int i, rc;
    unsigned long *max_lengts = calloc(1, sizeof(unsigned long) * fieldsLength);
    if (constructData(&(Stmt->stmt->result), data, dataLength, fieldsLength, max_lengts))
    {
        ma_free_root(fields_ma_alloc_root, MYF(0));
        ma_free_root(alloc_root, MYF(0));
        free(max_lengts);
        return MADB_SetError(&Stmt->Error, MADB_ERR_HY001, "Failed to allocate memory for data", 0);
    }
    for(i = 0; i < fieldsLength; i++) {
        if (constructField(fields_ma_alloc_root, &Stmt->stmt->fields[i], fields[i], fieldTypes[i], max_lengts[i]))
        {
            ma_free_root(fields_ma_alloc_root, MYF(0));
            ma_free_root(alloc_root, MYF(0));
            free(max_lengts);
            return MADB_SetError(&Stmt->Error, MADB_ERR_HY001, "Failed to allocate memory for fields", 0);
        }
    }
    free(max_lengts);
    Stmt->stmt->state = MYSQL_STMT_USE_OR_STORE_CALLED;
    Stmt->stmt->result_cursor = Stmt->stmt->result.data;
    Stmt->State = MADB_SS_PREPARED;
    MADB_InstallStmt(Stmt, Stmt->stmt);
    return SQL_SUCCESS;
}