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
#ifndef FOSSIL_DB_BLUECRAB_H
#define FOSSIL_DB_BLUECRAB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * CrabDB - Lightweight Embedded Relational Database
 * ============================================================
 * 
 * CrabDB is a lightweight, embedded relational database engine designed for
 * efficient data storage and retrieval in C applications. It provides a 
 * comprehensive API for managing databases, tables, records, and queries.
 *
 * Key Features:
 * - Multi-table database support with flexible schema definition
 * - Record-based storage with typed field values
 * - Query support for flexible data retrieval and filtering
 * - Transaction management for ACID compliance
 * - Comprehensive error handling and status reporting
 * - Support for multiple data types (integers, floats, strings, booleans, etc.)
 *
 * Primary Components:
 * - Database: Top-level container for tables and records
 * - Table: Structured collection of records with defined fields
 * - Record: Individual data row with typed field values
 * - Query: Advanced data retrieval and filtering mechanism
 * - Transaction: Atomic operations ensuring data consistency
 *
 * Usage Pattern:
 * 1. Create or open a database using fossil_db_crabdb_create()
 * 2. Create tables with fossil_db_crabdb_table_create()
 * 3. Insert records using fossil_db_crabdb_record_insert()
 * 4. Query data using fossil_db_crabdb_query_*() functions
 * 5. Manage transactions with fossil_db_crabdb_transaction_*() functions
 * 6. Close database with fossil_db_crabdb_close()
 *
 * ============================================================
 */

/* ============================================================
 * Version
 * ============================================================ */

#define FOSSIL_DB_CRABDB_VERSION_MAJOR 0
#define FOSSIL_DB_CRABDB_VERSION_MINOR 1
#define FOSSIL_DB_CRABDB_VERSION_PATCH 0

#define FOSSIL_DB_CRABDB_VERSION \
    "0.1.0"

/* ============================================================
 * Forward declarations
 * ============================================================ */

typedef struct fossil_db_crabdb_s
    fossil_db_crabdb_t;

typedef struct fossil_db_crabdb_table_s
    fossil_db_crabdb_table_t;

typedef struct fossil_db_crabdb_record_s
    fossil_db_crabdb_record_t;

typedef struct fossil_db_crabdb_field_s
    fossil_db_crabdb_field_t;

typedef struct fossil_db_crabdb_value_s
    fossil_db_crabdb_value_t;

typedef struct fossil_db_crabdb_query_s
    fossil_db_crabdb_query_t;

typedef struct fossil_db_crabdb_result_s
    fossil_db_crabdb_result_t;

typedef struct fossil_db_crabdb_transaction_s
    fossil_db_crabdb_transaction_t;

/* ============================================================
 * Status
 * ============================================================ */

typedef enum fossil_db_crabdb_status_e {
    FOSSIL_DB_CRABDB_SUCCESS = 0,

    FOSSIL_DB_CRABDB_ERROR,
    FOSSIL_DB_CRABDB_INVALID_ARGUMENT,
    FOSSIL_DB_CRABDB_OUT_OF_MEMORY,
    FOSSIL_DB_CRABDB_NOT_FOUND,
    FOSSIL_DB_CRABDB_ALREADY_EXISTS,
    FOSSIL_DB_CRABDB_EXISTS,
    FOSSIL_DB_CRABDB_INVALID_STATE,
    FOSSIL_DB_CRABDB_IO_ERROR,
    FOSSIL_DB_CRABDB_CORRUPTED,
    FOSSIL_DB_CRABDB_READ_ONLY,
    FOSSIL_DB_CRABDB_TRANSACTION_ERROR,
    FOSSIL_DB_CRABDB_QUERY_ERROR
} fossil_db_crabdb_status_t;

/* ============================================================
 * Value Types
 * ============================================================ */

typedef enum fossil_db_crabdb_type_e {
    FOSSIL_DB_CRABDB_TYPE_NULL = 0,

    FOSSIL_DB_CRABDB_TYPE_I8,
    FOSSIL_DB_CRABDB_TYPE_I16,
    FOSSIL_DB_CRABDB_TYPE_I32,
    FOSSIL_DB_CRABDB_TYPE_I64,

    FOSSIL_DB_CRABDB_TYPE_U8,
    FOSSIL_DB_CRABDB_TYPE_U16,
    FOSSIL_DB_CRABDB_TYPE_U32,
    FOSSIL_DB_CRABDB_TYPE_U64,

    FOSSIL_DB_CRABDB_TYPE_F32,
    FOSSIL_DB_CRABDB_TYPE_F64,

    FOSSIL_DB_CRABDB_TYPE_HEX,
    FOSSIL_DB_CRABDB_TYPE_OCT,
    FOSSIL_DB_CRABDB_TYPE_BIN,

    FOSSIL_DB_CRABDB_TYPE_CHAR,
    FOSSIL_DB_CRABDB_TYPE_CSTR,
    FOSSIL_DB_CRABDB_TYPE_BOOL,

    FOSSIL_DB_CRABDB_TYPE_SIZE,
    FOSSIL_DB_CRABDB_TYPE_DATETIME,
    FOSSIL_DB_CRABDB_TYPE_DURATION,

    FOSSIL_DB_CRABDB_TYPE_ANY
} fossil_db_crabdb_type_t;

/* ============================================================
 * Database
 * ============================================================ */

/**
 * Creates a new Blue Crab database at the specified path.
 *
 * This function initializes a new database handle and creates the backing
 * storage file if it does not already exist. The resulting handle is returned
 * through @p db and must be closed with fossil_db_crabdb_close() or released
 * with fossil_db_crabdb_destroy() when no longer needed.
 *
 * @param db Receives the newly created database handle on success.
 * @param path Filesystem path to the database file to create.
 * @return Status code indicating success or failure.
 */
fossil_db_crabdb_status_t
fossil_db_crabdb_create(
    fossil_db_crabdb_t **db,
    const char *path
);

/**
 * Opens an existing Blue Crab database stored on disk.
 *
 * The database is loaded from @p path and an initialized handle is returned in
 * @p db. If the path does not exist, the implementation may report a failure
 * status instead of creating a new database.
 *
 * @param db Receives the opened database handle on success.
 * @param path Path to the existing database file.
 * @return Status code indicating success or failure.
 */
fossil_db_crabdb_status_t
fossil_db_crabdb_open(
    fossil_db_crabdb_t **db,
    const char *path
);

/**
 * Creates a database in memory instead of on disk.
 *
 * This is useful for ephemeral or test-only workloads that do not need a
 * persistent file-backed store. The returned handle behaves like a regular
 * database handle, but all data is kept in volatile memory.
 *
 * @param db Receives the in-memory database handle on success.
 * @return Status code indicating success or failure.
 */
fossil_db_crabdb_status_t
fossil_db_crabdb_open_memory(
    fossil_db_crabdb_t **db
);

/**
 * Closes an open database and releases any active connection resources.
 *
 * After a successful close, the database handle should not be used again unless
 * it is reopened. This function is distinct from fossil_db_crabdb_destroy(),
 * which releases the handle object itself.
 *
 * @param db Database handle to close.
 * @return Status code indicating whether the close operation succeeded.
 */
fossil_db_crabdb_status_t
fossil_db_crabdb_close(
    fossil_db_crabdb_t *db
);

/**
 * Releases the database handle object itself.
 *
 * This function frees the memory associated with the handle and must be used
 * when a caller is done with the object allocated by the library. It is often
 * paired with fossil_db_crabdb_close() when a database was opened or created.
 *
 * @param db Database handle to destroy.
 */
void
fossil_db_crabdb_destroy(
    fossil_db_crabdb_t *db
);

/* ============================================================
 * Database Information
 * ============================================================ */

/**
 * Returns the Blue Crab library version string.
 *
 * The returned pointer is owned by the library and should not be freed by the
 * caller. Its format is implementation-defined but is commonly used for
 * compatibility checks and debugging output.
 *
 * @return Null-terminated version string.
 */
const char *
fossil_db_crabdb_version(void);

/**
 * Converts a status code into a readable textual description.
 *
 * This helper is intended for diagnostics, logging, and reporting errors in a
 * human-readable format.
 *
 * @param status Status code to translate.
 * @return Pointer to a static string describing the status.
 */
const char *
fossil_db_crabdb_status_string(
    fossil_db_crabdb_status_t status
);

/**
 * Retrieves the most recent error message associated with a database handle.
 *
 * The function stores the diagnostic text in @p message, allowing callers to
 * inspect the exact cause of a failed operation. If no error is recorded, the
 * message pointer may be null or empty depending on the implementation.
 *
 * @param db Database handle whose last error should be queried.
 * @param message Receives a pointer to the last error message string.
 * @return Status code indicating whether an error message was successfully
 *         retrieved.
 */
fossil_db_crabdb_status_t
fossil_db_crabdb_last_error(
    fossil_db_crabdb_t *db,
    const char **message
);

/* ============================================================
 * Tables
 * ============================================================ */

/**
 * Creates a new table in the database.
 *
 * The table name is used as an identifier within the database schema. If a
 * table with the same name already exists, the operation usually fails with a
 * duplicate-name status.
 *
 * @param db Database handle.
 * @param name Name of the table to create.
 * @return Status code indicating success or failure.
 */
fossil_db_crabdb_status_t
fossil_db_crabdb_create_table(
    fossil_db_crabdb_t *db,
    const char *name
);

/**
 * Removes an existing table and all of its contents from the database.
 *
 * This is a destructive operation. After a successful drop, the table name is
 * no longer available and any records contained within it are deleted.
 *
 * @param db Database handle.
 * @param name Name of the table to drop.
 * @return Status code indicating success or failure.
 */
fossil_db_crabdb_status_t
fossil_db_crabdb_drop_table(
    fossil_db_crabdb_t *db,
    const char *name
);

/**
 * Renames an existing table.
 *
 * This operation updates the logical name of a table while preserving the data
 * it contains. The new name must be valid and must not conflict with an
 * existing table in the database.
 *
 * @param db Database handle.
 * @param old_name Current table name.
 * @param new_name Desired replacement name.
 * @return Status code indicating success or failure.
 */
fossil_db_crabdb_status_t
fossil_db_crabdb_rename_table(
    fossil_db_crabdb_t *db,
    const char *old_name,
    const char *new_name
);

/**
 * Checks whether a table with the provided name exists in the database.
 *
 * This is a convenience helper for schema inspection and validation before
 * insert, select, or update operations.
 *
 * @param db Database handle.
 * @param name Name of the table to look up.
 * @return true if the table exists, otherwise false.
 */
bool
fossil_db_crabdb_table_exists(
    fossil_db_crabdb_t *db,
    const char *name
);

/* ============================================================
 * Records
 * ============================================================ */

/**
 * Inserts a new record into the specified table.
 *
 * The record structure is passed by pointer and is stored using the table's
 * schema or the database's internal logic. If the record violates constraints,
 * the insertion fails and the database returns an error status.
 *
 * @param db Database handle.
 * @param table Target table name.
 * @param record Record to insert.
 * @return Status code indicating success or failure.
 */
fossil_db_crabdb_status_t
fossil_db_crabdb_insert(
    fossil_db_crabdb_t *db,
    const char *table,
    fossil_db_crabdb_record_t *record
);

/**
 * Updates an existing record in the target table.
 *
 * This operation replaces the current contents of the record identified by the
 * provided record data or key with the updated values contained in @p record.
 *
 * @param db Database handle.
 * @param table Target table name.
 * @param record Updated record contents.
 * @return Status code indicating success or failure.
 */
fossil_db_crabdb_status_t
fossil_db_crabdb_update(
    fossil_db_crabdb_t *db,
    const char *table,
    fossil_db_crabdb_record_t *record
);

/**
 * Deletes an existing record from the specified table.
 *
 * The matching record is removed using its identifying information and the
 * database is updated accordingly. If no matching record is found, the call may
 * return a not-found status depending on the implementation.
 *
 * @param db Database handle.
 * @param table Table containing the record.
 * @param record Record identifying the row to delete.
 * @return Status code indicating success or failure.
 */
fossil_db_crabdb_status_t
fossil_db_crabdb_delete(
    fossil_db_crabdb_t *db,
    const char *table,
    fossil_db_crabdb_record_t *record
);

/**
 * Selects all records from a table and returns them as a result set.
 *
 * The returned result object contains the rows retrieved from @p table. The
 * caller is responsible for destroying the result using
 * fossil_db_crabdb_result_destroy() once it is no longer needed.
 *
 * @param db Database handle.
 * @param table Name of the table to query.
 * @param result Receives the result object containing matching records.
 * @return Status code indicating success or failure.
 */
fossil_db_crabdb_status_t
fossil_db_crabdb_select(
    fossil_db_crabdb_t *db,
    const char *table,
    fossil_db_crabdb_result_t **result
);

/* ============================================================
 * Values
 * ============================================================ */

/**
 * Allocates and initializes a value object with the specified type.
 *
 * The value is created as an empty container of the given type and is returned
 * in @p value for later assignment or use in records and query results.
 *
 * @param value Receives the newly created value instance.
 * @param type Data type to assign to the value.
 * @return Status code indicating success or failure.
 */
fossil_db_crabdb_status_t
fossil_db_crabdb_value_create(
    fossil_db_crabdb_value_t **value,
    fossil_db_crabdb_type_t type
);

/**
 * Releases memory associated with a value object.
 *
 * This function should be called for every value allocated via
 * fossil_db_crabdb_value_create() to avoid leaks.
 *
 * @param value Value to destroy.
 */
void
fossil_db_crabdb_value_destroy(
    fossil_db_crabdb_value_t *value
);

/**
 * Returns the runtime type tag of a value.
 *
 * This allows callers to safely inspect and interpret the payload stored within
 * a value object before reading or writing it.
 *
 * @param value Value whose type is requested.
 * @return Type enumeration for the value.
 */
fossil_db_crabdb_type_t
fossil_db_crabdb_value_type(
    const fossil_db_crabdb_value_t *value
);

/* ============================================================
 * Transactions
 * ============================================================ */

/**
 * Begins a transaction for the database.
 *
 * A transaction groups a sequence of operations so that they can be committed
 * together or rolled back atomically if an error occurs. Transaction semantics
 * depend on the underlying database implementation.
 *
 * @param db Database handle.
 * @return Status code indicating success or failure.
 */
fossil_db_crabdb_status_t
fossil_db_crabdb_begin(
    fossil_db_crabdb_t *db
);

/**
 * Commits the current transaction.
 *
 * If all operations in the transaction succeeded, this function makes the
 * changes permanent and closes the active transaction scope.
 *
 * @param db Database handle.
 * @return Status code indicating success or failure.
 */
fossil_db_crabdb_status_t
fossil_db_crabdb_commit(
    fossil_db_crabdb_t *db
);

/**
 * Rolls back the current transaction.
 *
 * Any changes made since the transaction began are discarded, restoring the
 * database to its previous state.
 *
 * @param db Database handle.
 * @return Status code indicating success or failure.
 */
fossil_db_crabdb_status_t
fossil_db_crabdb_rollback(
    fossil_db_crabdb_t *db
);

/* ============================================================
 * Results
 * ============================================================ */

/**
 * Returns the number of elements stored in a result object.
 *
 * This is a convenience function for iterating, validating, or reporting the
 * size of a result set without needing to inspect the internal structure.
 *
 * @param result Result set to inspect.
 * @return Number of rows or entries in the result.
 */
size_t
fossil_db_crabdb_result_count(
    const fossil_db_crabdb_result_t *result
);

/**
 * Releases memory allocated for a result set.
 *
 * Any result object returned by a query or select operation should be destroyed
 * when no longer required to prevent memory leaks.
 *
 * @param result Result object to destroy.
 */
void
fossil_db_crabdb_result_destroy(
    fossil_db_crabdb_result_t *result
);

#ifdef __cplusplus
}

#include <string>

namespace fossil {

    namespace database {

        class CrabDB {
        public:
            /**
             * Creates an empty CrabDB wrapper with no attached database handle.
             *
             * The object remains inactive until a database is created or opened
             * by calling create(), open(), or open_memory().
             */
            CrabDB()
                : db_(nullptr)
            {
            }

            /**
             * Creates a CrabDB wrapper and immediately opens or creates the
             * database at the given filesystem path.
             *
             * @param path Filesystem path to the database to create or open.
             */
            explicit CrabDB(const char *path)
                : db_(nullptr)
            {
                create(path);
            }

            /**
             * Creates a CrabDB wrapper and immediately opens or creates the
             * database at the given filesystem path.
             *
             * @param path Filesystem path to the database to create or open.
             */
            explicit CrabDB(const std::string &path)
                : db_(nullptr)
            {
                create(path);
            }

            /**
             * Destroys the database instance and releases any owned resources.
             *
             * The destructor calls destroy() to ensure the underlying native handle
             * is cleaned up before the wrapper goes out of scope.
             */
            ~CrabDB()
            {
                destroy();
            }

            /**
             * Returns the library version string for the underlying CrabDB
             * implementation.
             *
             * @return Null-terminated version string.
             */
            static const char *version()
            {
                return fossil_db_crabdb_version();
            }

            /**
             * Converts a CrabDB status code into a descriptive string.
             *
             * @param status Status value returned by a CrabDB operation.
             * @return Human-readable string describing the status code.
             */
            static const char *status_string(fossil_db_crabdb_status_t status)
            {
                return fossil_db_crabdb_status_string(status);
            }

            /**
             * Creates a new database at the supplied path and replaces any current
             * connection with the newly created one.
             *
             * Any previously opened or created database handle is destroyed before
             * the new database is initialized.
             *
             * @param path Path to the database file to create.
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t create(const char *path)
            {
                destroy();
                return fossil_db_crabdb_create(&db_, path);
            }

            /**
             * Creates a new database at the supplied path and replaces any current
             * connection with the newly created one.
             *
             * @param path Path to the database file to create.
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t create(const std::string &path)
            {
                destroy();
                return fossil_db_crabdb_create(&db_, path.c_str());
            }

            /**
             * Opens an existing database from the file system and replaces any
             * current connection.
             *
             * @param path Path to the database file to open.
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t open(const char *path)
            {
                destroy();
                return fossil_db_crabdb_open(&db_, path);
            }

            /**
             * Opens an existing database from the file system and replaces any
             * current connection.
             *
             * @param path Path to the database file to open.
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t open(const std::string &path)
            {
                destroy();
                return fossil_db_crabdb_open(&db_, path.c_str());
            }

            /**
             * Opens an in-memory database and replaces any current connection.
             *
             * This creates a transient database that exists only for the lifetime
             * of the current handle and is discarded when destroyed or closed.
             *
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t open_memory()
            {
                destroy();
                return fossil_db_crabdb_open_memory(&db_);
            }

            /**
             * Closes the currently open database connection without freeing the
             * wrapper object itself.
             *
             * After a successful close, the internal handle is set to null so the
             * object can be reused with another create/open call.
             *
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t close()
            {
                if (db_ == nullptr)
                    return FOSSIL_DB_CRABDB_INVALID_STATE;

                fossil_db_crabdb_status_t status = fossil_db_crabdb_close(db_);
                if (status == FOSSIL_DB_CRABDB_SUCCESS)
                    db_ = nullptr;
                return status;
            }

            /**
             * Releases the underlying native database handle and any associated
             * memory.
             *
             * This method is safe to call on an inactive object and resets the
             * stored pointer to null after destruction.
             */
            void destroy()
            {
                if (db_ != nullptr) {
                    fossil_db_crabdb_destroy(db_);
                    db_ = nullptr;
                }
            }

            /**
             * Retrieves the most recent error message associated with the current
             * database connection.
             *
             * @param message Pointer to receive the null-terminated error string.
             * @return Status code indicating whether the error message was fetched.
             */
            fossil_db_crabdb_status_t last_error(const char **message) const
            {
                if (db_ == nullptr)
                    return FOSSIL_DB_CRABDB_INVALID_STATE;
                return fossil_db_crabdb_last_error(db_, message);
            }

            /**
             * Creates a new table within the current database.
             *
             * @param name Name of the table to create.
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t create_table(const char *name)
            {
                return db_ == nullptr ? FOSSIL_DB_CRABDB_INVALID_STATE
                                    : fossil_db_crabdb_create_table(db_, name);
            }

            /**
             * Creates a new table within the current database.
             *
             * @param name Name of the table to create.
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t create_table(const std::string &name)
            {
                return db_ == nullptr ? FOSSIL_DB_CRABDB_INVALID_STATE
                                    : fossil_db_crabdb_create_table(db_, name.c_str());
            }

            /**
             * Removes an existing table from the current database.
             *
             * @param name Name of the table to drop.
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t drop_table(const char *name)
            {
                return db_ == nullptr ? FOSSIL_DB_CRABDB_INVALID_STATE
                                    : fossil_db_crabdb_drop_table(db_, name);
            }

            /**
             * Removes an existing table from the current database.
             *
             * @param name Name of the table to drop.
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t drop_table(const std::string &name)
            {
                return db_ == nullptr ? FOSSIL_DB_CRABDB_INVALID_STATE
                                    : fossil_db_crabdb_drop_table(db_, name.c_str());
            }

            /**
             * Renames an existing table while preserving its data.
             *
             * @param old_name Current table name.
             * @param new_name New table name to assign.
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t rename_table(const char *old_name, const char *new_name)
            {
                return db_ == nullptr ? FOSSIL_DB_CRABDB_INVALID_STATE
                                    : fossil_db_crabdb_rename_table(db_, old_name, new_name);
            }

            /**
             * Renames an existing table while preserving its data.
             *
             * @param old_name Current table name.
             * @param new_name New table name to assign.
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t rename_table(const std::string &old_name, const std::string &new_name)
            {
                return db_ == nullptr ? FOSSIL_DB_CRABDB_INVALID_STATE
                                    : fossil_db_crabdb_rename_table(db_, old_name.c_str(), new_name.c_str());
            }

            /**
             * Checks whether a named table exists in the current database.
             *
             * @param name Name of the table to inspect.
             * @return True if the table exists; otherwise false.
             */
            bool table_exists(const char *name) const
            {
                return db_ != nullptr && fossil_db_crabdb_table_exists(db_, name);
            }

            /**
             * Checks whether a named table exists in the current database.
             *
             * @param name Name of the table to inspect.
             * @return True if the table exists; otherwise false.
             */
            bool table_exists(const std::string &name) const
            {
                return db_ != nullptr && fossil_db_crabdb_table_exists(db_, name.c_str());
            }

            /**
             * Inserts a record into the specified table.
             *
             * The ownership and contents of the record are determined by the
             * underlying database layer and the supplied record instance.
             *
             * @param table Name of the table receiving the record.
             * @param record Record to insert.
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t insert(const char *table, fossil_db_crabdb_record_t *record)
            {
                return db_ == nullptr ? FOSSIL_DB_CRABDB_INVALID_STATE
                                    : fossil_db_crabdb_insert(db_, table, record);
            }

            /**
             * Inserts a record into the specified table.
             *
             * @param table Name of the table receiving the record.
             * @param record Record to insert.
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t insert(const std::string &table, fossil_db_crabdb_record_t *record)
            {
                return db_ == nullptr ? FOSSIL_DB_CRABDB_INVALID_STATE
                                    : fossil_db_crabdb_insert(db_, table.c_str(), record);
            }

            /**
             * Updates an existing record in the specified table.
             *
             * @param table Name of the table containing the record to update.
             * @param record Record containing the new values to write.
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t update(const char *table, fossil_db_crabdb_record_t *record)
            {
                return db_ == nullptr ? FOSSIL_DB_CRABDB_INVALID_STATE
                                    : fossil_db_crabdb_update(db_, table, record);
            }

            /**
             * Updates an existing record in the specified table.
             *
             * @param table Name of the table containing the record to update.
             * @param record Record containing the new values to write.
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t update(const std::string &table, fossil_db_crabdb_record_t *record)
            {
                return db_ == nullptr ? FOSSIL_DB_CRABDB_INVALID_STATE
                                    : fossil_db_crabdb_update(db_, table.c_str(), record);
            }

            /**
             * Deletes a record from the specified table.
             *
             * @param table Name of the table containing the record to remove.
             * @param record Record describing the row or key to delete.
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t delete_record(const char *table, fossil_db_crabdb_record_t *record)
            {
                return db_ == nullptr ? FOSSIL_DB_CRABDB_INVALID_STATE
                                    : fossil_db_crabdb_delete(db_, table, record);
            }

            /**
             * Deletes a record from the specified table.
             *
             * @param table Name of the table containing the record to remove.
             * @param record Record describing the row or key to delete.
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t delete_record(const std::string &table, fossil_db_crabdb_record_t *record)
            {
                return db_ == nullptr ? FOSSIL_DB_CRABDB_INVALID_STATE
                                    : fossil_db_crabdb_delete(db_, table.c_str(), record);
            }

            /**
             * Selects all rows from the given table and returns a result set.
             *
             * The caller is responsible for destroying the returned result object
             * when it is no longer needed.
             *
             * @param table Name of the table to query.
             * @param result Output pointer to receive the result set.
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t select(const char *table, fossil_db_crabdb_result_t **result)
            {
                return db_ == nullptr ? FOSSIL_DB_CRABDB_INVALID_STATE
                                    : fossil_db_crabdb_select(db_, table, result);
            }

            /**
             * Selects all rows from the given table and returns a result set.
             *
             * @param table Name of the table to query.
             * @param result Output pointer to receive the result set.
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t select(const std::string &table, fossil_db_crabdb_result_t **result)
            {
                return db_ == nullptr ? FOSSIL_DB_CRABDB_INVALID_STATE
                                    : fossil_db_crabdb_select(db_, table.c_str(), result);
            }

            /**
             * Starts a database transaction.
             *
             * Transactions allow multiple changes to be grouped and committed or
             * rolled back atomically as a single unit.
             *
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t begin()
            {
                return db_ == nullptr ? FOSSIL_DB_CRABDB_INVALID_STATE
                                    : fossil_db_crabdb_begin(db_);
            }

            /**
             * Commits the current transaction to the database.
             *
             * Any changes made since begin() are made permanent when this call
             * succeeds.
             *
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t commit()
            {
                return db_ == nullptr ? FOSSIL_DB_CRABDB_INVALID_STATE
                                    : fossil_db_crabdb_commit(db_);
            }

            /**
             * Rolls back the current transaction and discards all pending changes.
             *
             * This returns the database to its state at the beginning of the
             * transaction when the operation succeeds.
             *
             * @return Status code indicating success or failure.
             */
            fossil_db_crabdb_status_t rollback()
            {
                return db_ == nullptr ? FOSSIL_DB_CRABDB_INVALID_STATE
                                    : fossil_db_crabdb_rollback(db_);
            }

            /**
             * Returns the raw underlying native database handle.
             *
             * This accessor is intended for advanced integration and low-level
             * operations requiring direct access to the wrapped C API object.
             *
             * @return Pointer to the native CrabDB handle or null if no database is active.
             */
            fossil_db_crabdb_t *handle() const
            {
                return db_;
            }

        private:
            fossil_db_crabdb_t *db_;
        };
    
    } // namespace database

} // namespace fossil

#endif

#endif /* FOSSIL_DB_BLUECRAB_H */
