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

FOSSIL_SUITE(cpp_crabdb_fixture);

FOSSIL_SETUP(cpp_crabdb_fixture)
{
    // Setup the test fixture
}

FOSSIL_TEARDOWN(cpp_crabdb_fixture)
{
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST(cpp_test_crabdb_version_info)
{
    ASSUME_ITS_TRUE(fossil::database::CrabDB::version() != NULL);
    ASSUME_ITS_EQUAL_CSTR(fossil::database::CrabDB::version(), "0.1.0");
    ASSUME_ITS_TRUE(fossil::database::CrabDB::status_string(FOSSIL_DB_CRABDB_SUCCESS) != NULL);
    ASSUME_ITS_TRUE(fossil::database::CrabDB::status_string(FOSSIL_DB_CRABDB_NOT_FOUND) != NULL);

    fossil::database::CrabDB db;
    ASSUME_ITS_TRUE(db.handle() == NULL);
    ASSUME_ITS_TRUE(db.open_memory() == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.handle() != NULL);
    ASSUME_ITS_TRUE(db.close() == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.handle() == NULL);
}

FOSSIL_TEST(cpp_test_crabdb_create_open_close)
{
    const std::string file_name = "test_crabdb_cpp_basic.db";
    remove(file_name.c_str());
    fossil::database::CrabDB db;

    ASSUME_ITS_TRUE(db.create(file_name) == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.handle() != NULL);

    ASSUME_ITS_TRUE(db.close() == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.handle() == NULL);

    ASSUME_ITS_TRUE(db.open(file_name) == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.handle() != NULL);

    ASSUME_ITS_TRUE(db.close() == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.handle() == NULL);

    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_crabdb_table_and_value)
{
    const std::string file_name = "test_crabdb_cpp_table.db";
    remove(file_name.c_str());
    fossil_db_crabdb_value_t *value = NULL;
    fossil::database::CrabDB db;

    ASSUME_ITS_TRUE(db.create(file_name) == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.create_table("users") == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.table_exists("users") == true);

    ASSUME_ITS_TRUE(db.handle() != NULL);
    ASSUME_ITS_TRUE(fossil_db_crabdb_value_create(&value, FOSSIL_DB_CRABDB_TYPE_CSTR) == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(value != NULL);
    ASSUME_ITS_TRUE(fossil_db_crabdb_value_type(value) == FOSSIL_DB_CRABDB_TYPE_CSTR);

    fossil_db_crabdb_value_destroy(value);
    ASSUME_ITS_TRUE(db.close() == FOSSIL_DB_CRABDB_SUCCESS);
    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_crabdb_cpp_transaction_roundtrip)
{
    const std::string file_name = "test_crabdb_cpp_tx.db";
    remove(file_name.c_str());
    fossil::database::CrabDB db;

    ASSUME_ITS_TRUE(db.create(file_name) == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.begin() == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.create_table("orders") == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.commit() == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.close() == FOSSIL_DB_CRABDB_SUCCESS);

    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_crabdb_default_constructor_and_invalid_state)
{
    fossil::database::CrabDB db;

    ASSUME_ITS_TRUE(db.handle() == NULL);
    ASSUME_ITS_TRUE(db.close() == FOSSIL_DB_CRABDB_INVALID_STATE);
    ASSUME_ITS_TRUE(db.begin() == FOSSIL_DB_CRABDB_INVALID_STATE);
    ASSUME_ITS_TRUE(db.commit() == FOSSIL_DB_CRABDB_INVALID_STATE);
    ASSUME_ITS_TRUE(db.rollback() == FOSSIL_DB_CRABDB_INVALID_STATE);
    ASSUME_ITS_TRUE(db.create_table("missing") == FOSSIL_DB_CRABDB_INVALID_STATE);
    ASSUME_ITS_TRUE(db.table_exists("missing") == false);
}

FOSSIL_TEST(cpp_test_crabdb_rename_and_drop_table)
{
    const std::string file_name = "test_crabdb_cpp_rename.db";
    remove(file_name.c_str());
    fossil::database::CrabDB db;

    ASSUME_ITS_TRUE(db.create(file_name) == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.create_table("users") == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.table_exists("users") == true);

    ASSUME_ITS_TRUE(db.rename_table("users", "people") == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.table_exists("people") == true);
    ASSUME_ITS_TRUE(db.table_exists("users") == false);

    ASSUME_ITS_TRUE(db.drop_table("people") == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.table_exists("people") == false);

    ASSUME_ITS_TRUE(db.close() == FOSSIL_DB_CRABDB_SUCCESS);
    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_crabdb_transaction_rollback)
{
    const std::string file_name = "test_crabdb_cpp_tx_rollback.db";
    remove(file_name.c_str());
    fossil::database::CrabDB db;

    ASSUME_ITS_TRUE(db.create(file_name) == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.begin() == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.create_table("audit_log") == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.rollback() == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.table_exists("audit_log") == false);
    ASSUME_ITS_TRUE(db.close() == FOSSIL_DB_CRABDB_SUCCESS);

    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_crabdb_path_constructors_and_destroy)
{
    const std::string file_name = "test_crabdb_cpp_constructor.db";
    remove(file_name.c_str());

    {
        fossil::database::CrabDB db(file_name.c_str());
        ASSUME_ITS_TRUE(db.handle() != NULL);
        db.destroy();
        ASSUME_ITS_TRUE(db.handle() == NULL);
        db.destroy();
        ASSUME_ITS_TRUE(db.handle() == NULL);
    }

    {
        fossil::database::CrabDB db(file_name);
        ASSUME_ITS_TRUE(db.handle() != NULL);
        ASSUME_ITS_TRUE(db.close() == FOSSIL_DB_CRABDB_SUCCESS);
        ASSUME_ITS_TRUE(db.handle() == NULL);
    }

    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_crabdb_last_error_and_string_overloads)
{
    const std::string file_name = "test_crabdb_cpp_string_overloads.db";
    remove(file_name.c_str());
    const char *message = NULL;
    fossil::database::CrabDB db;

    ASSUME_ITS_TRUE(db.last_error(&message) == FOSSIL_DB_CRABDB_INVALID_STATE);
    ASSUME_ITS_TRUE(db.create(file_name) == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.create_table(std::string("items")) == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.table_exists(std::string("items")) == true);
    ASSUME_ITS_TRUE(db.rename_table(std::string("items"), std::string("products")) == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.table_exists(std::string("products")) == true);
    ASSUME_ITS_TRUE(db.drop_table(std::string("products")) == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(db.table_exists(std::string("products")) == false);
    ASSUME_ITS_TRUE(db.last_error(&message) == FOSSIL_DB_CRABDB_SUCCESS);
    ASSUME_ITS_TRUE(message != NULL);
    ASSUME_ITS_TRUE(db.close() == FOSSIL_DB_CRABDB_SUCCESS);

    remove(file_name.c_str());
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(cpp_crabdb_database_tests)
{
    FOSSIL_ADD_TEST(cpp_crabdb_fixture, cpp_test_crabdb_version_info);
    FOSSIL_ADD_TEST(cpp_crabdb_fixture, cpp_test_crabdb_create_open_close);
    FOSSIL_ADD_TEST(cpp_crabdb_fixture, cpp_test_crabdb_table_and_value);
    FOSSIL_ADD_TEST(cpp_crabdb_fixture, cpp_test_crabdb_cpp_transaction_roundtrip);
    FOSSIL_ADD_TEST(cpp_crabdb_fixture, cpp_test_crabdb_default_constructor_and_invalid_state);
    FOSSIL_ADD_TEST(cpp_crabdb_fixture, cpp_test_crabdb_rename_and_drop_table);
    FOSSIL_ADD_TEST(cpp_crabdb_fixture, cpp_test_crabdb_transaction_rollback);
    FOSSIL_ADD_TEST(cpp_crabdb_fixture, cpp_test_crabdb_path_constructors_and_destroy);
    FOSSIL_ADD_TEST(cpp_crabdb_fixture, cpp_test_crabdb_last_error_and_string_overloads);

    FOSSIL_ADD_SUITE(cpp_crabdb_fixture);
} // end of tests
