// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include "Microsoft/Schema/2_0/SystemReferenceStringTable.h"


namespace AppInstaller::Repository::Microsoft::Schema::V2_0
{
    namespace details
    {
        using namespace std::string_view_literals;

        struct NormalizedPackageNameTableInfo
        {
            inline static constexpr std::string_view TableName() { return "norm_names2"sv; }
            inline static constexpr std::string_view ValueName() { return "norm_name"sv; }
        };
    }

    // The table for Commands.
    using NormalizedPackageNameTable = SystemReferenceStringTable<details::NormalizedPackageNameTableInfo>;
}
