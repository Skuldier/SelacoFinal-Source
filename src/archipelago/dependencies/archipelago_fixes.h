#pragma once

// Fix for nlohmann::json namespace versioning
// The library uses versioned namespaces but we need the simple namespace

// First, include the json library
#include <nlohmann/json.hpp>

// Create namespace alias to handle versioned namespace
namespace nlohmann {
    #if !defined(NLOHMANN_JSON_NAMESPACE_NO_VERSION) || !NLOHMANN_JSON_NAMESPACE_NO_VERSION
        // The library uses versioned namespace, create an alias
        using json = NLOHMANN_JSON_NAMESPACE::json;
    #endif
}

// Ensure we're using the correct namespace
using json = nlohmann::json;
