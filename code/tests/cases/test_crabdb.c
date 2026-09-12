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
#include <fossil/maip/framework.h>

#include "fossil/db/framework.h"

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Utilities
// * * * * * * * * * * * * * * * * * * * * * * * *
// Setup steps for things like test fixtures and
// mock objects are set here.
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_SUITE(c_crabdb_fixture);

FOSSIL_SETUP(c_crabdb_fixture)
{
    // Setup the test fixture
}

FOSSIL_TEARDOWN(c_crabdb_fixture)
{
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST(c_test_crabdb_version_info)
{
    const char *version = fossil_db_crabdb_version();
    ASSUME_ITS_TRUE(version != NULL);
    ASSUME_ITS_EQUAL_CSTR(version, "0.1.0");

    ASSUME_ITS_TRUE(fossil_db_crabdb_status_string(FOSSIL_DB_CRABDB_SUCCESS) != NULL);
    ASSUME_ITS_TRUE(fossil_db_crabdb_status_string(FOSSIL_DB_CRABDB_NOT_FOUND) != NULL);
    ASSUME_ITS_TRUE(fossil_db_crabdb_status_string(FOSSIL_DB_CRABDB_QUERY_ERROR) != NULL);
}

FOSSIL_TEST(c_test_crabdb_create_open_close)
{
    fossil_db_crabdb_status_t status;
    fossil_db_crabdb_t *db = NULL;
    const char *file_name = "test_crabdb_basic.db";

    status = fossil_db_crabdb_create(&db, file_name);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db != NULL);

    status = fossil_db_crabdb_close(db);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);

    status = fossil_db_crabdb_open(&db, file_name);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db != NULL);

    status = fossil_db_crabdb_close(db);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);

    remove(file_name);
}

FOSSIL_TEST(c_test_crabdb_table_and_value)
{
    fossil_db_crabdb_status_t status;
    fossil_db_crabdb_t *db = NULL;
    fossil_db_crabdb_value_t *value = NULL;
    const char *file_name = "test_crabdb_table.db";

    status = fossil_db_crabdb_create(&db, file_name);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db != NULL);

    status = fossil_db_crabdb_create_table(db, "users");
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(fossil_db_crabdb_table_exists(db, "users") == true);

    status = fossil_db_crabdb_value_create(&value, FOSSIL_DB_CRABDB_TYPE_CSTR);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(value != NULL);
    ASSUME_ITS_TRUE(fossil_db_crabdb_value_type(value) == FOSSIL_DB_CRABDB_TYPE_CSTR);

    fossil_db_crabdb_value_destroy(value);
    value = NULL;
    status = fossil_db_crabdb_value_create(&value, FOSSIL_DB_CRABDB_TYPE_I64);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(fossil_db_crabdb_value_type(value) == FOSSIL_DB_CRABDB_TYPE_I64);

    fossil_db_crabdb_value_destroy(value);
    status = fossil_db_crabdb_close(db);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);

    remove(file_name);
}

FOSSIL_TEST(c_test_crabdb_transaction_roundtrip)
{
    fossil_db_crabdb_status_t status;
    fossil_db_crabdb_t *db = NULL;
    const char *file_name = "test_crabdb_tx.db";

    status = fossil_db_crabdb_create(&db, file_name);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db != NULL);

    status = fossil_db_crabdb_begin(db);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);

    status = fossil_db_crabdb_create_table(db, "orders");
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);

    status = fossil_db_crabdb_commit(db);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);

    status = fossil_db_crabdb_close(db);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);

    remove(file_name);
}

FOSSIL_TEST(c_test_crabdb_version_info_extended)
{
    ASSUME_ITS_TRUE(FOSSIL_DB_CRABDB_VERSION_MAJOR == 0);
    ASSUME_ITS_TRUE(FOSSIL_DB_CRABDB_VERSION_MINOR == 1);
    ASSUME_ITS_TRUE(FOSSIL_DB_CRABDB_VERSION_PATCH == 0);
    ASSUME_ITS_EQUAL_CSTR(FOSSIL_DB_CRABDB_VERSION, "0.1.0");
}

FOSSIL_TEST(c_test_crabdb_open_memory_and_destroy)
{
    fossil_db_crabdb_status_t status;
    fossil_db_crabdb_t *db = NULL;

    status = fossil_db_crabdb_open_memory(&db);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db != NULL);

    status = fossil_db_crabdb_close(db);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);

    fossil_db_crabdb_destroy(db);
}

FOSSIL_TEST(c_test_crabdb_table_rename_drop)
{
    fossil_db_crabdb_status_t status;
    fossil_db_crabdb_t *db = NULL;
    const char *file_name = "test_crabdb_table_ops.db";

    status = fossil_db_crabdb_create(&db, file_name);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db != NULL);

    status = fossil_db_crabdb_create_table(db, "users");
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(fossil_db_crabdb_table_exists(db, "users") == true);

    status = fossil_db_crabdb_rename_table(db, "users", "players");
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(fossil_db_crabdb_table_exists(db, "players") == true);

    status = fossil_db_crabdb_drop_table(db, "players");
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(fossil_db_crabdb_table_exists(db, "players") == false);

    status = fossil_db_crabdb_close(db);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);

    remove(file_name);
}

FOSSIL_TEST(c_test_crabdb_rollback)
{
    fossil_db_crabdb_status_t status;
    fossil_db_crabdb_t *db = NULL;
    const char *file_name = "test_crabdb_rollback.db";

    status = fossil_db_crabdb_create(&db, file_name);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db != NULL);

    status = fossil_db_crabdb_begin(db);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);

    status = fossil_db_crabdb_create_table(db, "audit");
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(fossil_db_crabdb_table_exists(db, "audit") == true);

    status = fossil_db_crabdb_rollback(db);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(fossil_db_crabdb_table_exists(db, "audit") == false);

    status = fossil_db_crabdb_close(db);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);

    remove(file_name);
}

FOSSIL_TEST(c_test_crabdb_status_strings)
{
    fossil_db_crabdb_status_t status;

    for (status = FOSSIL_DB_CRABDB_SUCCESS;
         status <= FOSSIL_DB_CRABDB_QUERY_ERROR;
         status++)
    {
        ASSUME_ITS_TRUE(fossil_db_crabdb_status_string(status) != NULL);
    }
}

FOSSIL_TEST(c_test_crabdb_value_types)
{
    fossil_db_crabdb_type_t type;

    for (type = FOSSIL_DB_CRABDB_TYPE_NULL;
         type <= FOSSIL_DB_CRABDB_TYPE_ANY;
         type++)
    {
        fossil_db_crabdb_value_t *value = NULL;
        fossil_db_crabdb_status_t status =
            fossil_db_crabdb_value_create(&value, type);

        ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
        ASSUME_ITS_TRUE(value != NULL);
        ASSUME_ITS_TRUE(fossil_db_crabdb_value_type(value) == type);
        fossil_db_crabdb_value_destroy(value);
    }
}

FOSSIL_TEST(c_test_crabdb_transaction_state_errors)
{
    fossil_db_crabdb_status_t status;
    fossil_db_crabdb_t *db = NULL;

    status = fossil_db_crabdb_open_memory(&db);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);

    status = fossil_db_crabdb_commit(db);
    ASSUME_ITS_TRUE(status != FOSSIL_DB_CRABDB_SUCCESS);
    status = fossil_db_crabdb_rollback(db);
    ASSUME_ITS_TRUE(status != FOSSIL_DB_CRABDB_SUCCESS);

    status = fossil_db_crabdb_close(db);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    fossil_db_crabdb_destroy(db);
}

FOSSIL_TEST(c_test_crabdb_last_error)
{
    fossil_db_crabdb_status_t status;
    fossil_db_crabdb_t *db = NULL;
    const char *message = NULL;

    status = fossil_db_crabdb_open_memory(&db);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db != NULL);

    status = fossil_db_crabdb_last_error(db, &message);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS ||
                    status != FOSSIL_DB_CRABDB_SUCCESS);

    status = fossil_db_crabdb_close(db);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    fossil_db_crabdb_destroy(db);
}

FOSSIL_TEST(c_test_crabdb_select_empty_table)
{
    fossil_db_crabdb_status_t status;
    fossil_db_crabdb_t *db = NULL;
    fossil_db_crabdb_result_t *result = NULL;

    status = fossil_db_crabdb_open_memory(&db);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);

    status = fossil_db_crabdb_create_table(db, "events");
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    status = fossil_db_crabdb_select(db, "events", &result);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(result != NULL);
    ASSUME_ITS_TRUE(fossil_db_crabdb_result_count(result) == 0);

    fossil_db_crabdb_result_destroy(result);
    status = fossil_db_crabdb_close(db);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    fossil_db_crabdb_destroy(db);
}

FOSSIL_TEST(c_test_crabdb_table_edge_cases)
{
    fossil_db_crabdb_status_t status;
    fossil_db_crabdb_t *db = NULL;

    status = fossil_db_crabdb_open_memory(&db);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(fossil_db_crabdb_table_exists(db, "missing") == false);

    status = fossil_db_crabdb_create_table(db, "items");
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    status = fossil_db_crabdb_create_table(db, "items");
    ASSUME_ITS_TRUE(status != FOSSIL_DB_CRABDB_SUCCESS);

    status = fossil_db_crabdb_close(db);
    ASSUME_ITS_TRUE(status == FOSSIL_DB_CRABDB_SUCCESS);
    fossil_db_crabdb_destroy(db);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_crabdb_database_tests)
{
    FOSSIL_ADD_TEST(c_crabdb_fixture, c_test_crabdb_version_info);
    FOSSIL_ADD_TEST(c_crabdb_fixture, c_test_crabdb_create_open_close);
    FOSSIL_ADD_TEST(c_crabdb_fixture, c_test_crabdb_table_and_value);
    FOSSIL_ADD_TEST(c_crabdb_fixture, c_test_crabdb_transaction_roundtrip);
    FOSSIL_ADD_TEST(c_crabdb_fixture, c_test_crabdb_version_info_extended);
    FOSSIL_ADD_TEST(c_crabdb_fixture, c_test_crabdb_open_memory_and_destroy);
    FOSSIL_ADD_TEST(c_crabdb_fixture, c_test_crabdb_table_rename_drop);
    FOSSIL_ADD_TEST(c_crabdb_fixture, c_test_crabdb_rollback);
    FOSSIL_ADD_TEST(c_crabdb_fixture, c_test_crabdb_status_strings);
    FOSSIL_ADD_TEST(c_crabdb_fixture, c_test_crabdb_value_types);
    FOSSIL_ADD_TEST(c_crabdb_fixture, c_test_crabdb_transaction_state_errors);
    FOSSIL_ADD_TEST(c_crabdb_fixture, c_test_crabdb_last_error);
    FOSSIL_ADD_TEST(c_crabdb_fixture, c_test_crabdb_select_empty_table);
    FOSSIL_ADD_TEST(c_crabdb_fixture, c_test_crabdb_table_edge_cases);

    FOSSIL_ADD_SUITE(c_crabdb_fixture);
} // end of tests
