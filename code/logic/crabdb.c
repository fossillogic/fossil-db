/**
 * -----------------------------------------------------------------------------
 * Project: Fossil Logic
 *
 * This file is part of the Fossil Logic project, which aims to develop
 * high-performance, cross-platform applications and libraries. The code
 * contained herein is licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * Author: Michael Gene Brockus (Dreamer)
 * Date: 04/05/2013
 *
 * Copyright (C) 2013-Current Fossil Logic. All rights reserved.
 * -----------------------------------------------------------------------------
 */
#include "fossil/db/crabdb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 * Internal Constants
 * ============================================================ */

#define FOSSIL_DB_CRABDB_INITIAL_TABLE_CAPACITY 8
#define FOSSIL_DB_CRABDB_MAX_ERROR_LENGTH 256

/* ============================================================
 * Internal Structures
 * ============================================================ */

/* ============================================================
 * Internal Table
 * ============================================================ */

struct fossil_db_crabdb_table_s
{
    char *name;

    fossil_db_crabdb_field_t **fields;
    size_t field_count;
    size_t field_capacity;

    fossil_db_crabdb_record_t **records;
    size_t record_count;
    size_t record_capacity;

    uint64_t next_record_id;
};

/* ============================================================
 * Internal Record
 * ============================================================ */

struct fossil_db_crabdb_record_s
{
    uint64_t id;

    char **names;
    fossil_db_crabdb_value_t **values;
    size_t value_count;
    size_t value_capacity;

    fossil_db_crabdb_table_t *table;
};

/* ============================================================
 * Internal Field
 * ============================================================ */

struct fossil_db_crabdb_field_s
{
    char *name;

    fossil_db_crabdb_type_t type;

    size_t offset;
    size_t size;

    bool nullable;
    bool primary_key;
    bool unique;
};

/* ============================================================
 * Internal Value
 * ============================================================ */

struct fossil_db_crabdb_value_s
{
    fossil_db_crabdb_type_t type;

    void *data;
    size_t size;
    size_t capacity;

    bool owned;
};

/* ============================================================
 * Internal Query
 * ============================================================ */

struct fossil_db_crabdb_query_s
{
    char *query;

    size_t length;
    size_t position;

    size_t error_line;
    size_t error_column;

    fossil_db_crabdb_status_t status;
};

/* ============================================================
 * Internal Result
 * ============================================================ */

struct fossil_db_crabdb_result_s
{
    fossil_db_crabdb_record_t **records;

    size_t count;
    size_t capacity;

    size_t position;
};

/* ============================================================
 * Internal Transaction
 * ============================================================ */

struct fossil_db_crabdb_transaction_s
{
    bool active;

    size_t depth;

    fossil_db_crabdb_table_t **tables;
    size_t table_count;
};

/* ============================================================
 * Internal Database
 * ============================================================ */

struct fossil_db_crabdb_s
{
    char *path;

    bool memory;
    bool closed;
    bool read_only;
    bool transaction_active;

    fossil_db_crabdb_table_t **tables;
    size_t table_count;
    size_t table_capacity;
    size_t transaction_table_count;

    fossil_db_crabdb_transaction_t *transaction;

    size_t affected_rows;

    fossil_db_crabdb_status_t status;

    char error[FOSSIL_DB_CRABDB_MAX_ERROR_LENGTH];
};

/* ============================================================
 * Internal Helpers
 * ============================================================ */

static char *
crabdb_strdup(const char *string)
{
    size_t length;
    char *copy;

    if (string == NULL)
    {
        return NULL;
    }

    length = strlen(string);

    copy = malloc(length + 1);

    if (copy == NULL)
    {
        return NULL;
    }

    memcpy(copy, string, length + 1);

    return copy;
}

static void
crabdb_set_error(
    fossil_db_crabdb_t *db,
    const char *message)
{
    if (db == NULL)
    {
        return;
    }

    if (message == NULL)
    {
        db->error[0] = '\0';
        return;
    }

    snprintf(
        db->error,
        sizeof(db->error),
        "%s",
        message);
}

static bool
crabdb_valid_db(
    fossil_db_crabdb_t *db)
{
    if (db == NULL)
    {
        return false;
    }

    if (db->closed)
    {
        return false;
    }

    return true;
}

static fossil_db_crabdb_table_t *
crabdb_find_table(
    fossil_db_crabdb_t *db,
    const char *name)
{
    size_t i;

    if (db == NULL || name == NULL || db->tables == NULL)
    {
        return NULL;
    }

    for (i = 0; i < db->table_count; ++i)
    {
        if (db->tables[i] == NULL || db->tables[i]->name == NULL)
        {
            continue;
        }

        if (strcmp(db->tables[i]->name, name) == 0)
        {
            return db->tables[i];
        }
    }

    return NULL;
}

static fossil_db_crabdb_status_t
crabdb_grow_tables(
    fossil_db_crabdb_t *db)
{
    size_t capacity;
    fossil_db_crabdb_table_t **tables;

    if (db == NULL)
    {
        return FOSSIL_DB_CRABDB_INVALID_ARGUMENT;
    }

    capacity = db->table_capacity == 0
                   ? FOSSIL_DB_CRABDB_INITIAL_TABLE_CAPACITY
                   : db->table_capacity * 2;

    tables = realloc(
        db->tables,
        sizeof(*tables) * capacity);

    if (tables == NULL)
    {
        crabdb_set_error(
            db,
            "Unable to allocate table registry.");

        return FOSSIL_DB_CRABDB_OUT_OF_MEMORY;
    }

    db->tables = tables;
    db->table_capacity = capacity;

    return FOSSIL_DB_CRABDB_SUCCESS;
}

static fossil_db_crabdb_status_t
crabdb_allocate(
    fossil_db_crabdb_t **db,
    const char *path,
    bool memory)
{
    fossil_db_crabdb_t *instance;

    if (db == NULL)
    {
        return FOSSIL_DB_CRABDB_INVALID_ARGUMENT;
    }

    *db = NULL;

    instance = calloc(
        1,
        sizeof(*instance));

    if (instance == NULL)
    {
        return FOSSIL_DB_CRABDB_OUT_OF_MEMORY;
    }

    instance->memory = memory;
    instance->closed = false;
    instance->transaction_active = false;
    instance->transaction_table_count = 0;

    if (path != NULL)
    {
        instance->path = crabdb_strdup(path);

        if (instance->path == NULL)
        {
            free(instance);
            return FOSSIL_DB_CRABDB_OUT_OF_MEMORY;
        }
    }

    instance->table_capacity =
        FOSSIL_DB_CRABDB_INITIAL_TABLE_CAPACITY;

    instance->tables = calloc(
        instance->table_capacity,
        sizeof(*instance->tables));

    if (instance->tables == NULL)
    {
        free(instance->path);
        free(instance);

        return FOSSIL_DB_CRABDB_OUT_OF_MEMORY;
    }

    instance->error[0] = '\0';

    *db = instance;

    return FOSSIL_DB_CRABDB_SUCCESS;
}

static void
crabdb_free_record(fossil_db_crabdb_record_t *record)
{
    size_t i;

    if (record == NULL)
    {
        return;
    }

    for (i = 0; i < record->value_count; ++i)
    {
        free(record->names[i]);
        fossil_db_crabdb_value_destroy(record->values[i]);
    }

    free(record->names);
    free(record->values);
    free(record);
}

static void
crabdb_free_table(fossil_db_crabdb_table_t *table)
{
    size_t i;

    if (table == NULL)
    {
        return;
    }

    for (i = 0; i < table->record_count; ++i)
    {
        crabdb_free_record(table->records[i]);
    }

    free(table->records);
    free(table->fields);
    free(table->name);
    free(table);
}

static bool
crabdb_read(FILE *file, void *data, size_t size)
{
    return size == 0 || fread(data, 1, size, file) == size;
}

static char *
crabdb_read_string(FILE *file)
{
    size_t length;
    char *string;

    if (!crabdb_read(file, &length, sizeof(length)) ||
        length == 0 || length > (1024 * 1024))
        return NULL;

    string = malloc(length);
    if (string == NULL || !crabdb_read(file, string, length) ||
        string[length - 1] != '\0')
    {
        free(string);
        return NULL;
    }
    return string;
}

static bool
crabdb_load(fossil_db_crabdb_t *db)
{
    FILE *file;
    uint32_t magic;
    size_t table_count, i, j, k;

    if (db->memory || db->path == NULL)
        return true;
    file = fopen(db->path, "rb");
    if (file == NULL || !crabdb_read(file, &magic, sizeof(magic)) ||
        magic != 0x31424443 || !crabdb_read(file, &table_count, sizeof(table_count)) ||
        table_count > 100000)
    {
        if (file != NULL)
            fclose(file);
        return false;
    }

    for (i = 0; i < table_count; ++i)
    {
        char *name = crabdb_read_string(file);
        size_t field_count, record_count;
        fossil_db_crabdb_table_t *table;
        if (name == NULL || !crabdb_read(file, &field_count, sizeof(field_count)) ||
            field_count > 100000 ||
            fossil_db_crabdb_create_table(db, name) != FOSSIL_DB_CRABDB_SUCCESS)
        {
            free(name);
            fclose(file);
            return false;
        }
        free(name);
        table = db->tables[db->table_count - 1];
        for (j = 0; j < field_count; ++j)
        {
            fossil_db_crabdb_type_t type;
            size_t offset, size;
            bool nullable, primary_key, unique;
            fossil_db_crabdb_field_t *field;
            char *field_name = crabdb_read_string(file);
            if (field_name == NULL || !crabdb_read(file, &type, sizeof(type)) ||
                !crabdb_read(file, &offset, sizeof(offset)) ||
                !crabdb_read(file, &size, sizeof(size)) ||
                !crabdb_read(file, &nullable, sizeof(nullable)) ||
                !crabdb_read(file, &primary_key, sizeof(primary_key)) ||
                !crabdb_read(file, &unique, sizeof(unique)))
            {
                free(field_name);
                fclose(file);
                return false;
            }

            field = calloc(1, sizeof(*field));
            if (field == NULL)
            {
                free(field_name);
                fclose(file);
                return false;
            }
            field->name = field_name;
            field->type = type;
            field->offset = offset;
            field->size = size;
            field->nullable = nullable;
            field->primary_key = primary_key;
            field->unique = unique;

            if (table->field_count >= table->field_capacity)
            {
                size_t capacity = table->field_capacity == 0
                                      ? FOSSIL_DB_CRABDB_INITIAL_TABLE_CAPACITY
                                      : table->field_capacity * 2;
                fossil_db_crabdb_field_t **fields = realloc(
                    table->fields, sizeof(*fields) * capacity);
                if (fields == NULL)
                {
                    free(field->name);
                    free(field);
                    fclose(file);
                    return false;
                }
                table->fields = fields;
                table->field_capacity = capacity;
            }
            table->fields[table->field_count++] = field;
        }
        if (!crabdb_read(file, &record_count, sizeof(record_count)) || record_count > 1000000)
        {
            fclose(file);
            return false;
        }
        for (j = 0; j < record_count; ++j)
        {
            fossil_db_crabdb_record_t *record = calloc(1, sizeof(*record));
            if (record == NULL || !crabdb_read(file, &record->id, sizeof(record->id)) ||
                !crabdb_read(file, &record->value_count, sizeof(record->value_count)) ||
                record->value_count > 100000)
            {
                crabdb_free_record(record);
                fclose(file);
                return false;
            }
            record->table = table;
            record->value_capacity = record->value_count;
            if (record->value_count != 0)
            {
                record->names = calloc(record->value_count, sizeof(*record->names));
                record->values = calloc(record->value_count, sizeof(*record->values));
                if (record->names == NULL || record->values == NULL)
                {
                    crabdb_free_record(record);
                    fclose(file);
                    return false;
                }
            }
            for (k = 0; k < record->value_count; ++k)
            {
                size_t size;
                record->names[k] = crabdb_read_string(file);
                record->values[k] = calloc(1, sizeof(*record->values[k]));
                if (record->names[k] == NULL || record->values[k] == NULL ||
                    !crabdb_read(file, &record->values[k]->type, sizeof(record->values[k]->type)) ||
                    !crabdb_read(file, &size, sizeof(size)) || size > 1024 * 1024)
                {
                    crabdb_free_record(record);
                    fclose(file);
                    return false;
                }
                record->values[k]->size = record->values[k]->capacity = size;
                if (size != 0 && !(record->values[k]->data = malloc(size)))
                {
                    crabdb_free_record(record);
                    fclose(file);
                    return false;
                }
                if (!crabdb_read(file, record->values[k]->data, size))
                {
                    crabdb_free_record(record);
                    fclose(file);
                    return false;
                }
            }
            if (record->id > table->next_record_id)
                table->next_record_id = record->id;
            table->records = realloc(table->records, sizeof(*table->records) * (table->record_count + 1));
            if (table->records == NULL)
            {
                crabdb_free_record(record);
                fclose(file);
                return false;
            }
            table->records[table->record_count++] = record;
        }
    }
    fclose(file);
    return true;
}

/* Persist the in-memory graph in a pointer-free private format. */
static bool
crabdb_save(fossil_db_crabdb_t *db)
{
    FILE *file;
    uint32_t magic = 0x31424443; /* CDB1 */
    size_t i, j, k;

    if (db->memory || db->path == NULL)
        return true;

    file = fopen(db->path, "wb");
    if (file == NULL)
        return false;

    if (fwrite(&magic, sizeof(magic), 1, file) != 1 ||
        fwrite(&db->table_count, sizeof(db->table_count), 1, file) != 1)
    {
        fclose(file);
        return false;
    }

    for (i = 0; i < db->table_count; ++i)
    {
        fossil_db_crabdb_table_t *table = db->tables[i];
        size_t length = strlen(table->name) + 1;

        if (fwrite(&length, sizeof(length), 1, file) != 1 ||
            fwrite(table->name, 1, length, file) != length ||
            fwrite(&table->field_count, sizeof(table->field_count), 1, file) != 1)
        {
            fclose(file);
            return false;
        }

        for (j = 0; j < table->field_count; ++j)
        {
            fossil_db_crabdb_field_t *field = table->fields[j];
            length = strlen(field->name) + 1;
            if (fwrite(&length, sizeof(length), 1, file) != 1 ||
                fwrite(field->name, 1, length, file) != length ||
                fwrite(&field->type, sizeof(field->type), 1, file) != 1 ||
                fwrite(&field->offset, sizeof(field->offset), 1, file) != 1 ||
                fwrite(&field->size, sizeof(field->size), 1, file) != 1 ||
                fwrite(&field->nullable, sizeof(field->nullable), 1, file) != 1 ||
                fwrite(&field->primary_key, sizeof(field->primary_key), 1, file) != 1 ||
                fwrite(&field->unique, sizeof(field->unique), 1, file) != 1)
            {
                fclose(file);
                return false;
            }
        }

        if (fwrite(&table->record_count, sizeof(table->record_count), 1, file) != 1)
        {
            fclose(file);
            return false;
        }
        for (j = 0; j < table->record_count; ++j)
        {
            fossil_db_crabdb_record_t *record = table->records[j];
            if (fwrite(&record->id, sizeof(record->id), 1, file) != 1 ||
                fwrite(&record->value_count, sizeof(record->value_count), 1, file) != 1)
            {
                fclose(file);
                return false;
            }
            for (k = 0; k < record->value_count; ++k)
            {
                fossil_db_crabdb_value_t *value = record->values[k];
                length = strlen(record->names[k]) + 1;
                if (fwrite(&length, sizeof(length), 1, file) != 1 ||
                    fwrite(record->names[k], 1, length, file) != length ||
                    fwrite(&value->type, sizeof(value->type), 1, file) != 1 ||
                    fwrite(&value->size, sizeof(value->size), 1, file) != 1 ||
                    (value->size != 0 && fwrite(value->data, 1, value->size, file) != value->size))
                {
                    fclose(file);
                    return false;
                }
            }
        }
    }

    return fclose(file) == 0;
}

/* ============================================================
 * Version
 * ============================================================ */

const char *
fossil_db_crabdb_version(void)
{
    return FOSSIL_DB_CRABDB_VERSION;
}

/* ============================================================
 * Status
 * ============================================================ */

const char *
fossil_db_crabdb_status_string(
    fossil_db_crabdb_status_t status)
{
    switch (status)
    {
    case FOSSIL_DB_CRABDB_SUCCESS:
        return "success";

    case FOSSIL_DB_CRABDB_ERROR:
        return "error";

    case FOSSIL_DB_CRABDB_INVALID_ARGUMENT:
        return "invalid argument";

    case FOSSIL_DB_CRABDB_OUT_OF_MEMORY:
        return "out of memory";

    case FOSSIL_DB_CRABDB_NOT_FOUND:
        return "not found";

    case FOSSIL_DB_CRABDB_ALREADY_EXISTS:
        return "already exists";

    case FOSSIL_DB_CRABDB_EXISTS:
        return "exists";

    case FOSSIL_DB_CRABDB_INVALID_STATE:
        return "invalid state";

    case FOSSIL_DB_CRABDB_IO_ERROR:
        return "I/O error";

    case FOSSIL_DB_CRABDB_CORRUPTED:
        return "database corrupted";

    case FOSSIL_DB_CRABDB_READ_ONLY:
        return "read only";

    case FOSSIL_DB_CRABDB_TRANSACTION_ERROR:
        return "transaction error";

    case FOSSIL_DB_CRABDB_QUERY_ERROR:
        return "query error";

    default:
        return "unknown status";
    }
}

/* ============================================================
 * Database
 * ============================================================ */

fossil_db_crabdb_status_t
fossil_db_crabdb_create(
    fossil_db_crabdb_t **db,
    const char *path)
{
    fossil_db_crabdb_status_t status;
    FILE *file;

    if (db == NULL || path == NULL || path[0] == '\0')
    {
        return FOSSIL_DB_CRABDB_INVALID_ARGUMENT;
    }

    file = fopen(path, "wb");

    if (file == NULL)
    {
        return FOSSIL_DB_CRABDB_IO_ERROR;
    }

    fclose(file);

    status = crabdb_allocate(
        db,
        path,
        false);

    return status;
}

fossil_db_crabdb_status_t
fossil_db_crabdb_open(
    fossil_db_crabdb_t **db,
    const char *path)
{
    FILE *file;

    if (db == NULL || path == NULL || path[0] == '\0')
    {
        return FOSSIL_DB_CRABDB_INVALID_ARGUMENT;
    }

    file = fopen(path, "rb");

    if (file == NULL)
    {
        return FOSSIL_DB_CRABDB_NOT_FOUND;
    }

    fclose(file);

    if (crabdb_allocate(db, path, false) != FOSSIL_DB_CRABDB_SUCCESS)
        return FOSSIL_DB_CRABDB_OUT_OF_MEMORY;
    if (!crabdb_load(*db))
    {
        fossil_db_crabdb_destroy(*db);
        *db = NULL;
        return FOSSIL_DB_CRABDB_CORRUPTED;
    }
    return FOSSIL_DB_CRABDB_SUCCESS;
}

fossil_db_crabdb_status_t
fossil_db_crabdb_open_memory(
    fossil_db_crabdb_t **db)
{
    return crabdb_allocate(
        db,
        NULL,
        true);
}

fossil_db_crabdb_status_t
fossil_db_crabdb_close(
    fossil_db_crabdb_t *db)
{
    size_t i;

    if (db == NULL)
    {
        return FOSSIL_DB_CRABDB_INVALID_ARGUMENT;
    }

    if (db->closed)
    {
        return FOSSIL_DB_CRABDB_INVALID_STATE;
    }

    if (!crabdb_save(db))
    {
        crabdb_set_error(db, "Unable to persist database.");
        return FOSSIL_DB_CRABDB_IO_ERROR;
    }

    for (i = 0; i < db->table_count; ++i)
    {
        if (db->tables[i] == NULL)
        {
            continue;
        }

        crabdb_free_table(db->tables[i]);
    }

    free(db->tables);

    db->tables = NULL;
    db->table_count = 0;
    db->table_capacity = 0;

    db->closed = true;

    return FOSSIL_DB_CRABDB_SUCCESS;
}

void fossil_db_crabdb_destroy(
    fossil_db_crabdb_t *db)
{
    if (db == NULL)
    {
        return;
    }

    if (!db->closed)
    {
        fossil_db_crabdb_close(db);
    }

    free(db->path);
    free(db);
}

/* ============================================================
 * Database Information
 * ============================================================ */

fossil_db_crabdb_status_t
fossil_db_crabdb_last_error(
    fossil_db_crabdb_t *db,
    const char **message)
{
    if (db == NULL || message == NULL)
    {
        return FOSSIL_DB_CRABDB_INVALID_ARGUMENT;
    }

    *message = db->error;

    return FOSSIL_DB_CRABDB_SUCCESS;
}

/* ============================================================
 * Tables
 * ============================================================ */

fossil_db_crabdb_status_t
fossil_db_crabdb_create_table(
    fossil_db_crabdb_t *db,
    const char *name)
{
    fossil_db_crabdb_table_t *table;
    fossil_db_crabdb_status_t status;

    if (!crabdb_valid_db(db) || name == NULL || name[0] == '\0')
    {
        return FOSSIL_DB_CRABDB_INVALID_ARGUMENT;
    }

    if (crabdb_find_table(db, name) != NULL)
    {
        crabdb_set_error(
            db,
            "Table already exists.");

        return FOSSIL_DB_CRABDB_ALREADY_EXISTS;
    }

    if (db->table_count >= db->table_capacity)
    {
        status = crabdb_grow_tables(db);

        if (status != FOSSIL_DB_CRABDB_SUCCESS)
        {
            return status;
        }
    }

    table = calloc(
        1,
        sizeof(*table));

    if (table == NULL)
    {
        crabdb_set_error(
            db,
            "Unable to allocate table.");

        return FOSSIL_DB_CRABDB_OUT_OF_MEMORY;
    }

    table->name = crabdb_strdup(name);

    if (table->name == NULL)
    {
        free(table);

        crabdb_set_error(
            db,
            "Unable to allocate table name.");

        return FOSSIL_DB_CRABDB_OUT_OF_MEMORY;
    }

    db->tables[db->table_count++] = table;

    return FOSSIL_DB_CRABDB_SUCCESS;
}

fossil_db_crabdb_status_t
fossil_db_crabdb_drop_table(
    fossil_db_crabdb_t *db,
    const char *name)
{
    size_t i;
    fossil_db_crabdb_table_t *table;

    if (!crabdb_valid_db(db) || name == NULL)
    {
        return FOSSIL_DB_CRABDB_INVALID_ARGUMENT;
    }

    for (i = 0; i < db->table_count; ++i)
    {
        table = db->tables[i];

        if (table == NULL)
        {
            continue;
        }

        if (strcmp(table->name, name) != 0)
        {
            continue;
        }

        crabdb_free_table(table);

        for (; i + 1 < db->table_count; ++i)
        {
            db->tables[i] = db->tables[i + 1];
        }

        db->tables[db->table_count - 1] = NULL;
        db->table_count--;

        return FOSSIL_DB_CRABDB_SUCCESS;
    }

    crabdb_set_error(
        db,
        "Table was not found.");

    return FOSSIL_DB_CRABDB_NOT_FOUND;
}

fossil_db_crabdb_status_t
fossil_db_crabdb_rename_table(
    fossil_db_crabdb_t *db,
    const char *old_name,
    const char *new_name)
{
    fossil_db_crabdb_table_t *table;
    char *name;

    if (!crabdb_valid_db(db) ||
        old_name == NULL ||
        new_name == NULL ||
        old_name[0] == '\0' ||
        new_name[0] == '\0')
    {
        return FOSSIL_DB_CRABDB_INVALID_ARGUMENT;
    }

    table = crabdb_find_table(
        db,
        old_name);

    if (table == NULL)
    {
        crabdb_set_error(
            db,
            "Table was not found.");

        return FOSSIL_DB_CRABDB_NOT_FOUND;
    }

    if (crabdb_find_table(db, new_name) != NULL)
    {
        crabdb_set_error(
            db,
            "Destination table already exists.");

        return FOSSIL_DB_CRABDB_ALREADY_EXISTS;
    }

    name = crabdb_strdup(new_name);

    if (name == NULL)
    {
        return FOSSIL_DB_CRABDB_OUT_OF_MEMORY;
    }

    free(table->name);
    table->name = name;

    return FOSSIL_DB_CRABDB_SUCCESS;
}

bool fossil_db_crabdb_table_exists(
    fossil_db_crabdb_t *db,
    const char *name)
{
    if (!crabdb_valid_db(db) || name == NULL)
    {
        return false;
    }

    return crabdb_find_table(
               db,
               name) != NULL;
}

/* ============================================================
 * Records
 * ============================================================ */

static fossil_db_crabdb_status_t
crabdb_grow_records(
    fossil_db_crabdb_table_t *table)
{
    size_t capacity;
    fossil_db_crabdb_record_t **records;

    capacity = table->record_capacity == 0
                   ? FOSSIL_DB_CRABDB_INITIAL_TABLE_CAPACITY
                   : table->record_capacity * 2;

    records = realloc(table->records, sizeof(*records) * capacity);

    if (records == NULL)
    {
        return FOSSIL_DB_CRABDB_OUT_OF_MEMORY;
    }

    table->records = records;
    table->record_capacity = capacity;

    return FOSSIL_DB_CRABDB_SUCCESS;
}

fossil_db_crabdb_status_t
fossil_db_crabdb_insert(
    fossil_db_crabdb_t *db,
    const char *table,
    fossil_db_crabdb_record_t *record)
{
    if (!crabdb_valid_db(db) ||
        table == NULL ||
        record == NULL)
    {
        return FOSSIL_DB_CRABDB_INVALID_ARGUMENT;
    }

    if (!fossil_db_crabdb_table_exists(db, table))
    {
        crabdb_set_error(
            db,
            "Insert target table was not found.");

        return FOSSIL_DB_CRABDB_NOT_FOUND;
    }

    {
        fossil_db_crabdb_table_t *instance = crabdb_find_table(db, table);

        if (instance->record_count >= instance->record_capacity)
        {
            fossil_db_crabdb_status_t status = crabdb_grow_records(instance);

            if (status != FOSSIL_DB_CRABDB_SUCCESS)
            {
                crabdb_set_error(db, "Unable to allocate record storage.");
                return status;
            }
        }

        record->id = ++instance->next_record_id;
        record->table = instance;
        instance->records[instance->record_count++] = record;
        db->affected_rows = 1;
    }

    return FOSSIL_DB_CRABDB_SUCCESS;
}

fossil_db_crabdb_status_t
fossil_db_crabdb_update(
    fossil_db_crabdb_t *db,
    const char *table,
    fossil_db_crabdb_record_t *record)
{
    if (!crabdb_valid_db(db) ||
        table == NULL ||
        record == NULL)
    {
        return FOSSIL_DB_CRABDB_INVALID_ARGUMENT;
    }

    if (!fossil_db_crabdb_table_exists(db, table))
    {
        return FOSSIL_DB_CRABDB_NOT_FOUND;
    }

    {
        fossil_db_crabdb_table_t *instance = crabdb_find_table(db, table);
        size_t i;

        for (i = 0; i < instance->record_count; ++i)
        {
            if (instance->records[i]->id == record->id)
            {
                if (instance->records[i] != record)
                    crabdb_free_record(instance->records[i]);

                record->table = instance;
                instance->records[i] = record;
                db->affected_rows = 1;
                return FOSSIL_DB_CRABDB_SUCCESS;
            }
        }
    }

    return FOSSIL_DB_CRABDB_NOT_FOUND;
}

fossil_db_crabdb_status_t
fossil_db_crabdb_delete(
    fossil_db_crabdb_t *db,
    const char *table,
    fossil_db_crabdb_record_t *record)
{
    if (!crabdb_valid_db(db) ||
        table == NULL ||
        record == NULL)
    {
        return FOSSIL_DB_CRABDB_INVALID_ARGUMENT;
    }

    if (!fossil_db_crabdb_table_exists(db, table))
    {
        return FOSSIL_DB_CRABDB_NOT_FOUND;
    }

    {
        fossil_db_crabdb_table_t *instance = crabdb_find_table(db, table);
        size_t i;

        for (i = 0; i < instance->record_count; ++i)
        {
            if (instance->records[i] == record ||
                instance->records[i]->id == record->id)
            {
                crabdb_free_record(instance->records[i]);

                for (; i + 1 < instance->record_count; ++i)
                    instance->records[i] = instance->records[i + 1];

                instance->records[--instance->record_count] = NULL;
                db->affected_rows = 1;
                return FOSSIL_DB_CRABDB_SUCCESS;
            }
        }
    }

    return FOSSIL_DB_CRABDB_NOT_FOUND;
}

fossil_db_crabdb_status_t
fossil_db_crabdb_select(
    fossil_db_crabdb_t *db,
    const char *table,
    fossil_db_crabdb_result_t **result)
{
    if (!crabdb_valid_db(db) ||
        table == NULL ||
        result == NULL)
    {
        return FOSSIL_DB_CRABDB_INVALID_ARGUMENT;
    }

    *result = NULL;

    if (!fossil_db_crabdb_table_exists(db, table))
    {
        return FOSSIL_DB_CRABDB_NOT_FOUND;
    }

    {
        fossil_db_crabdb_table_t *instance = crabdb_find_table(db, table);
        fossil_db_crabdb_result_t *output;

        output = calloc(1, sizeof(*output));

        if (output == NULL)
            return FOSSIL_DB_CRABDB_OUT_OF_MEMORY;

        if (instance->record_count != 0)
        {
            output->records = malloc(
                sizeof(*output->records) * instance->record_count);

            if (output->records == NULL)
            {
                free(output);
                return FOSSIL_DB_CRABDB_OUT_OF_MEMORY;
            }

            memcpy(
                output->records,
                instance->records,
                sizeof(*output->records) * instance->record_count);
        }

        output->count = instance->record_count;
        output->capacity = instance->record_count;
        *result = output;
    }

    return FOSSIL_DB_CRABDB_SUCCESS;
}

/* ============================================================
 * Values
 * ============================================================ */

fossil_db_crabdb_status_t
fossil_db_crabdb_value_create(
    fossil_db_crabdb_value_t **value,
    fossil_db_crabdb_type_t type)
{
    fossil_db_crabdb_value_t *instance;

    if (value == NULL)
    {
        return FOSSIL_DB_CRABDB_INVALID_ARGUMENT;
    }

    *value = NULL;

    instance = calloc(
        1,
        sizeof(*instance));

    if (instance == NULL)
    {
        return FOSSIL_DB_CRABDB_OUT_OF_MEMORY;
    }

    instance->type = type;

    *value = instance;

    return FOSSIL_DB_CRABDB_SUCCESS;
}

void fossil_db_crabdb_value_destroy(
    fossil_db_crabdb_value_t *value)
{
    if (value == NULL)
    {
        return;
    }

    free(value->data);
    free(value);
}

fossil_db_crabdb_type_t
fossil_db_crabdb_value_type(
    const fossil_db_crabdb_value_t *value)
{
    if (value == NULL)
    {
        return FOSSIL_DB_CRABDB_TYPE_NULL;
    }

    return value->type;
}

/* ============================================================
 * Transactions
 * ============================================================ */

fossil_db_crabdb_status_t
fossil_db_crabdb_begin(
    fossil_db_crabdb_t *db)
{
    if (!crabdb_valid_db(db))
    {
        return FOSSIL_DB_CRABDB_INVALID_ARGUMENT;
    }

    if (db->transaction_active)
    {
        crabdb_set_error(
            db,
            "Transaction is already active.");

        return FOSSIL_DB_CRABDB_TRANSACTION_ERROR;
    }

    db->transaction_active = true;
    db->transaction_table_count = db->table_count;

    return FOSSIL_DB_CRABDB_SUCCESS;
}

fossil_db_crabdb_status_t
fossil_db_crabdb_commit(
    fossil_db_crabdb_t *db)
{
    if (!crabdb_valid_db(db))
    {
        return FOSSIL_DB_CRABDB_INVALID_ARGUMENT;
    }

    if (!db->transaction_active)
    {
        crabdb_set_error(
            db,
            "No active transaction.");

        return FOSSIL_DB_CRABDB_TRANSACTION_ERROR;
    }

    db->transaction_active = false;

    return FOSSIL_DB_CRABDB_SUCCESS;
}

fossil_db_crabdb_status_t
fossil_db_crabdb_rollback(
    fossil_db_crabdb_t *db)
{
    size_t i;

    if (!crabdb_valid_db(db))
    {
        return FOSSIL_DB_CRABDB_INVALID_ARGUMENT;
    }

    if (!db->transaction_active)
    {
        crabdb_set_error(
            db,
            "No active transaction.");

        return FOSSIL_DB_CRABDB_TRANSACTION_ERROR;
    }

    /* Discard tables created after the transaction began. */
    for (i = db->transaction_table_count; i < db->table_count; ++i)
    {
        crabdb_free_table(db->tables[i]);
        db->tables[i] = NULL;
    }

    db->table_count = db->transaction_table_count;
    db->transaction_active = false;

    return FOSSIL_DB_CRABDB_SUCCESS;
}

/* ============================================================
 * Results
 * ============================================================ */

size_t
fossil_db_crabdb_result_count(
    const fossil_db_crabdb_result_t *result)
{
    if (result == NULL)
    {
        return 0;
    }

    return result->count;
}

void fossil_db_crabdb_result_destroy(
    fossil_db_crabdb_result_t *result)
{
    if (result == NULL)
    {
        return;
    }

    free(result);
}
