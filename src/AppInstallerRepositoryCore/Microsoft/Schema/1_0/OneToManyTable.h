// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include <winget/SQLiteWrapper.h>
#include <string>
#include <string_view>
#include <vector>


namespace AppInstaller::Repository::Microsoft::Schema::V1_0
{
    // Allow the different schema version to indicate which they are.
    enum class OneToManyTableSchema
    {
        // Does not used a named unique index for either table.
        // Map table has primary key and rowid.
        Version_1_0,
        // Uses a named unique index for both tables.
        // Map table has rowid.
        Version_1_1,
        // Uses a named unique index for data table. (No change from 1.1)
        // Map table has primary key and no rowid.
        Version_1_7,
    };

    namespace details
    {
        // Returns the map table name for a given table.
        std::string OneToManyTableGetMapTableName(std::string_view tableName);

        // Returns the manifest column name.
        std::string_view OneToManyTableGetManifestColumnName();

        // Create the tables.
        void CreateOneToManyTable(SQLite::Connection& connection, OneToManyTableSchema schemaVersion, std::string_view tableName, std::string_view valueName);

        // Drop the tables.
        void DropOneToManyTable(SQLite::Connection& connection, std::string_view tableName);

        // Gets all values associated with the given manifest id.
        std::vector<std::string> OneToManyTableGetValuesByManifestId(
            const SQLite::Connection& connection,
            std::string_view tableName,
            std::string_view valueName,
            SQLite::rowid_t manifestId);

        // Ensures that the value exists and inserts mapping entries.
        void OneToManyTableEnsureExistsAndInsert(SQLite::Connection& connection,
            std::string_view tableName, std::string_view valueName, 
            const std::vector<Utility::NormalizedString>& values, SQLite::rowid_t manifestId);

        // Updates the mapping table to represent the given values for the manifest.
        bool OneToManyTableUpdateIfNeededByManifestId(SQLite::Connection& connection,
            std::string_view tableName, std::string_view valueName,
            const std::vector<Utility::NormalizedString>& values, SQLite::rowid_t manifestId);

        // Deletes the mapping rows for the given manifest, then removes any unused data rows.
        void OneToManyTableDeleteIfNotNeededByManifestId(SQLite::Connection& connection, std::string_view tableName, std::string_view valueName, SQLite::rowid_t manifestId);

        // Removes data that is no longer needed for an index that is to be published.
        void OneToManyTablePrepareForPackaging(SQLite::Connection& connection, std::string_view tableName, OneToManyTableSchema schemaVersion, bool preserveManifestIndex, bool preserveValuesIndex);

        // Checks the consistency of the index to ensure that every referenced row exists.
        // Returns true if index is consistent; false if it is not.
         bool OneToManyTableCheckConsistency(const SQLite::Connection& connection, std::string_view tableName, std::string_view valueName, bool log);

        // Determines if the table is empty.
        bool OneToManyTableIsEmpty(SQLite::Connection& connection, std::string_view tableName);

        // Prepares a statement for replacing all of the map data to point to a single manifest for a given id.
        SQLite::Statement OneToManyTablePrepareMapDataFoldingStatement(const SQLite::Connection& connection, std::string_view tableName);
    }

    // Gets the manifest id that the PrepareMapDataFoldingStatement will fold to for the given manifest id.
    std::optional<SQLite::rowid_t> OneToManyTableGetMapDataFoldingManifestTargetId(const SQLite::Connection& connection, SQLite::rowid_t manifestId);

    // A table that represents a value that is 1:N with a primary entry.
    template <typename TableInfo>
    struct OneToManyTable
    {
        // The name of the table.
        static constexpr std::string_view TableName()
        {
            return TableInfo::TableName();
        }

        // The value name of the table.
        static constexpr std::string_view ValueName()
        {
            return TableInfo::ValueName();
        }

        // Value indicating type.
        static constexpr bool IsOneToOne()
        {
            return false;
        }

        // Creates the table with named indices.
        static void Create(SQLite::Connection& connection, OneToManyTableSchema schemaVersion)
        {
            details::CreateOneToManyTable(connection, schemaVersion, TableInfo::TableName(), TableInfo::ValueName());
        }

        // Drops the table.
        static void Drop(SQLite::Connection& connection)
        {
            details::DropOneToManyTable(connection, TableInfo::TableName());
        }

        // Gets all values associated with the given manifest id.
        static std::vector<std::string> GetValuesByManifestId(const SQLite::Connection& connection, SQLite::rowid_t manifestId)
        {
            return details::OneToManyTableGetValuesByManifestId(connection, TableInfo::TableName(), TableInfo::ValueName(), manifestId);
        }

        // Ensures that all values exist in the data table, and inserts into the mapping table for the given manifest id.
        static void EnsureExistsAndInsert(SQLite::Connection& connection, const std::vector<Utility::NormalizedString>& values, SQLite::rowid_t manifestId)
        {
            details::OneToManyTableEnsureExistsAndInsert(connection, TableInfo::TableName(), TableInfo::ValueName(), values, manifestId);
        }

        // Updates the mapping table to represent the given values for the manifest.
        static bool UpdateIfNeededByManifestId(SQLite::Connection& connection, const std::vector<Utility::NormalizedString>& values, SQLite::rowid_t manifestId)
        {
            return details::OneToManyTableUpdateIfNeededByManifestId(connection, TableInfo::TableName(), TableInfo::ValueName(), values, manifestId);
        }

        // Deletes the mapping rows for the given manifest, then removes any unused data rows.
        static void DeleteIfNotNeededByManifestId(SQLite::Connection& connection, SQLite::rowid_t manifestId)
        {
            details::OneToManyTableDeleteIfNotNeededByManifestId(connection, TableInfo::TableName(), TableInfo::ValueName(), manifestId);
        }

        // Removes data that is no longer needed for an index that is to be published.
        // Preserving the manifest index will improve the efficiency of finding the values associated with a manifest.
        // Preserving the values index will improve searching when it is primarily done by equality.
        static void PrepareForPackaging(SQLite::Connection& connection, OneToManyTableSchema schemaVersion, bool preserveManifestIndex, bool preserveValuesIndex)
        {
            details::OneToManyTablePrepareForPackaging(connection, TableInfo::TableName(), schemaVersion, preserveManifestIndex, preserveValuesIndex);
        }

        // Checks the consistency of the index to ensure that every referenced row exists.
        // Returns true if index is consistent; false if it is not.
        static bool CheckConsistency(const SQLite::Connection& connection, bool log)
        {
            return details::OneToManyTableCheckConsistency(connection, TableInfo::TableName(), TableInfo::ValueName(), log);
        }

        // Determines if the table is empty.
        static bool IsEmpty(SQLite::Connection& connection)
        {
            return details::OneToManyTableIsEmpty(connection, TableInfo::TableName());
        }

        // Prepares a statement for replacing all of the map data to point to a single manifest for a given id.
        static SQLite::Statement PrepareMapDataFoldingStatement(const SQLite::Connection& connection)
        {
            return details::OneToManyTablePrepareMapDataFoldingStatement(connection, TableInfo::TableName());
        }
    };
}
