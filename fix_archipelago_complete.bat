@echo off
setlocal enabledelayedexpansion

echo ================================================================================
echo Archipelago Complete Auto-Patcher for Selaco
echo ================================================================================
echo.

:: Check if we're in the right directory
if not exist "src\archipelago" (
    echo ERROR: Cannot find src\archipelago directory!
    echo Please run this script from the Selaco root directory.
    pause
    exit /b 1
)

echo [1/8] Creating required directories...
if not exist "src\archipelago\dependencies" mkdir "src\archipelago\dependencies"
if not exist "src\archipelago\dependencies\wswrap\include" mkdir "src\archipelago\dependencies\wswrap\include"

echo [2/8] Creating archipelago_fixes.h...
(
echo #pragma once
echo.
echo // Fix for nlohmann::json namespace versioning
echo #include ^<nlohmann/json.hpp^>
echo.
echo namespace nlohmann {
echo     #if !defined^(NLOHMANN_JSON_NAMESPACE_NO_VERSION^) ^|^| !NLOHMANN_JSON_NAMESPACE_NO_VERSION
echo         using json = NLOHMANN_JSON_NAMESPACE::json;
echo     #endif
echo }
echo.
echo using json = nlohmann::json;
) > "src\archipelago\dependencies\archipelago_fixes.h"

echo [3/8] Creating wswrap.hpp...
(
echo #pragma once
echo #include ^<string^>
echo #include ^<functional^>
echo #include ^<memory^>
echo.
echo namespace wswrap {
echo class WebSocketClient {
echo public:
echo     WebSocketClient^(^) = default;
echo     virtual ~WebSocketClient^(^) = default;
echo     std::function^<void^(^)^> on_open;
echo     std::function^<void^(const std::string^&^)^> on_message;
echo     std::function^<void^(int, const std::string^&^)^> on_close;
echo     std::function^<void^(const std::string^&^)^> on_error;
echo     virtual bool connect^(const std::string^& url^) = 0;
echo     virtual void disconnect^(^) = 0;
echo     virtual bool send^(const std::string^& message^) = 0;
echo     virtual bool is_connected^(^) const = 0;
echo };
echo std::unique_ptr^<WebSocketClient^> create_websocket_client^(^);
echo }
) > "src\archipelago\dependencies\wswrap\include\wswrap.hpp"

echo [4/8] Creating apclient.hpp stub...
(
echo #pragma once
echo #include "../archipelago_fixes.h"
echo #include ^<string^>
echo #include ^<vector^>
echo #include ^<functional^>
echo.
echo class APClient {
echo public:
echo     struct NetworkItem {
echo         int64_t item;
echo         int64_t location;
echo         int player;
echo         int flags;
echo     };
echo     
echo     APClient^(const std::string^& uuid, const std::string^& game, const std::string^& uri^) {}
echo     ~APClient^(^) {}
echo     
echo     void poll^(^) {}
echo     void ConnectSlot^(const std::string^& name, const std::string^& password, int items_handling, 
echo                      const std::vector^<std::string^>^& tags, const std::tuple^<int,int,int^>^& version^) {}
echo     void LocationChecks^(const std::vector^<int64_t^>^& locations^) {}
echo     void Say^(const std::string^& text^) {}
echo     
echo     // Callbacks
echo     void set_socket_connected_handler^(std::function^<void^(^)^> f^) {}
echo     void set_socket_disconnected_handler^(std::function^<void^(^)^> f^) {}
echo     void set_slot_connected_handler^(std::function^<void^(const json^&^)^> f^) {}
echo     void set_slot_refused_handler^(std::function^<void^(const std::vector^<std::string^>^&^)^> f^) {}
echo     void set_items_received_handler^(std::function^<void^(const std::vector^<NetworkItem^>^&^)^> f^) {}
echo     void set_print_json_handler^(std::function^<void^(const json^&^)^> f^) {}
echo };
) > "src\archipelago\dependencies\apclient.hpp"

echo [5/8] Creating apuuid.hpp stub...
(
echo #pragma once
echo #include ^<string^>
echo #include ^<chrono^>
echo.
echo inline std::string ap_get_uuid^(const std::string^& game_name^) {
echo     // Simple UUID generation for now
echo     return game_name + "_" + std::to_string^(std::chrono::system_clock::now^(^).time_since_epoch^(^).count^(^)^);
echo }
) > "src\archipelago\dependencies\apuuid.hpp"

echo [6/8] Fixing source file includes...

:: Fix ap_manager.cpp
powershell -Command "(Get-Content 'src\archipelago\core\ap_manager.cpp' -Raw) -replace '#include <nlohmann/json.hpp>', '#include \"\"../dependencies/archipelago_fixes.h\"\"' -replace '\"\"', '\"' | Set-Content 'src\archipelago\core\ap_manager.cpp' -NoNewline"

:: Fix ap_manager.h
powershell -Command "$content = Get-Content 'src\archipelago\core\ap_manager.h' -Raw; if ($content -notmatch 'archipelago_fixes.h') { $content = $content -replace '(#include \"\"ap_types.h\"\")', \"\"$1`n#include `\"../dependencies/archipelago_fixes.h`\"`n\"\"; $content = $content -replace '\"\"\""', '\"' | Set-Content 'src\archipelago\core\ap_manager.h' -NoNewline }"

:: Fix ap_protocol.cpp
powershell -Command "$content = Get-Content 'src\archipelago\core\ap_protocol.cpp' -Raw; if ($content -notmatch 'archipelago_fixes.h') { $content = $content -replace '(#include \"\"ap_protocol.h\"\")', \"\"$1`n#include `\"../dependencies/archipelago_fixes.h`\"`n\"\"; $content = $content -replace '\"\"\""', '\"' | Set-Content 'src\archipelago\core\ap_protocol.cpp' -NoNewline }"

:: Fix ap_protocol.h
powershell -Command "(Get-Content 'src\archipelago\core\ap_protocol.h' -Raw) -replace '#include <nlohmann/json.hpp>', '#include \"\"../dependencies/archipelago_fixes.h\"\"' -replace '\"\"', '\"' | Set-Content 'src\archipelago\core\ap_protocol.h' -NoNewline"

:: Fix ap_network.cpp
powershell -Command "$content = Get-Content 'src\archipelago\core\ap_network.cpp' -Raw; $content = $content -replace '#include \"\"apclient.hpp\"\"', '#include \"\"../dependencies/apclient.hpp\"\"'; $content = $content -replace '#include \"\"apuuid.hpp\"\"', '#include \"\"../dependencies/apuuid.hpp\"\"'; $content = $content -replace '#include <nlohmann/json.hpp>', '#include \"\"../dependencies/archipelago_fixes.h\"\"'; $content = $content -replace '\"\"\""', '\"' | Set-Content 'src\archipelago\core\ap_network.cpp' -NoNewline"

:: Fix ap_network.h  
powershell -Command "$content = Get-Content 'src\archipelago\core\ap_network.h' -Raw; if ($content -notmatch 'archipelago_fixes.h') { $content = $content -replace '(// Forward declaration of APClient)', \"\"#include `\"../dependencies/archipelago_fixes.h`\"`n`n$1\"\"; $content = $content -replace '\"\"\""', '\"' | Set-Content 'src\archipelago\core\ap_network.h' -NoNewline }"

:: Fix ap_state.cpp
powershell -Command "(Get-Content 'src\archipelago\core\ap_state.cpp' -Raw) -replace '#include <nlohmann/json.hpp>', '#include \"\"../dependencies/archipelago_fixes.h\"\"' -replace '\"\"', '\"' | Set-Content 'src\archipelago\core\ap_state.cpp' -NoNewline"

:: Fix ap_overlay.cpp includes
powershell -Command "(Get-Content 'src\archipelago\ui\ap_overlay.cpp' -Raw) -replace 'common/2d/v_text.h', 'common/fonts/v_text.h' | Set-Content 'src\archipelago\ui\ap_overlay.cpp' -NoNewline"

:: Add missing include to ap_overlay.h
powershell -Command "$content = Get-Content 'src\archipelago\ui\ap_overlay.h' -Raw; if ($content -notmatch '<vector>') { $content = $content -replace '(#include \"\"../core/ap_types.h\"\")', \"\"$1`n#include ^<vector^>\"\"; $content = $content -replace '\"\"\""', '\"' | Set-Content 'src\archipelago\ui\ap_overlay.h' -NoNewline }"

:: Add missing includes to ap_network.cpp
powershell -Command "$content = Get-Content 'src\archipelago\core\ap_network.cpp' -Raw; if ($content -notmatch '<chrono>') { $content = $content -replace '(#include \"\"ap_network.h\"\")', \"\"$1`n#include ^<chrono^>`n#include ^<tuple^>\"\"; $content = $content -replace '\"\"\""', '\"' | Set-Content 'src\archipelago\core\ap_network.cpp' -NoNewline }"

echo [7/8] Updating CMakeLists.txt...

:: Backup CMakeLists.txt
copy "src\CMakeLists.txt" "src\CMakeLists.txt.backup" >nul 2>&1

:: Add namespace fix to CMakeLists.txt
powershell -Command "$content = Get-Content 'src\CMakeLists.txt' -Raw; if ($content -notmatch 'NLOHMANN_JSON_NAMESPACE_NO_VERSION') { $content = $content -replace '(add_definitions\(-DASIO_STANDALONE=1\))', \"\"$1`n    `n    # FIX: Disable JSON versioned namespace to avoid conflicts`n    add_definitions(-DNLOHMANN_JSON_NAMESPACE_NO_VERSION=1)\"\"; $content = $content -replace '\"\"\""', '\"' | Set-Content 'src\CMakeLists.txt' -NoNewline }"

echo [8/8] Creating stub implementations...

:: Create ap_network_impl.cpp if needed
if not exist "src\archipelago\core\ap_network_impl.cpp" (
echo Creating network implementation stub...
(
echo #include "ap_network.h"
echo.
echo namespace Archipelago {
echo // Stub implementations for missing methods
echo void APNetworkClient::SendLocationScouts^(const std::vector^<int64_t^>^& locations^) {}
echo void APNetworkClient::SendBounce^(const json^& data^) {}
echo void APNetworkClient::SendStatusUpdate^(int status^) {}
echo void APNetworkClient::GetData^(const std::vector^<std::string^>^& keys^) {}
echo void APNetworkClient::SetData^(const std::string^& key, const json^& value^) {}
echo void APNetworkClient::StopNetworkThread^(^) {}
echo }
) > "src\archipelago\core\ap_network_impl.cpp"
)

:: Create ap_manager_impl.cpp if needed
if not exist "src\archipelago\core\ap_manager_impl.cpp" (
echo Creating manager implementation stub...
(
echo #include "ap_manager.h"
echo.
echo namespace Archipelago {
echo // Stub implementations for missing methods
echo void Manager::ScoutLocation^(int64_t location_id^) {}
echo void Manager::ScoutLocations^(const std::vector^<int64_t^>^& location_ids^) {}
echo void Manager::SendDeathLink^(const std::string^& cause^) {}
echo void Manager::GetData^(const std::vector^<std::string^>^& keys^) {}
echo void Manager::SetData^(const std::string^& key, const json^& value^) {}
echo void Manager::SetGoalComplete^(^) {}
echo void Manager::SetGameComplete^(^) {}
echo void Manager::SetDeathLinkCallback^(DeathLinkCallback callback^) {}
echo std::vector^<Message^> Manager::GetAllMessages^(^) { return {}; }
echo const NetworkPlayer* Manager::GetPlayer^(int slot^) const { return nullptr; }
echo const NetworkPlayer* Manager::GetLocalPlayer^(^) const { return nullptr; }
echo void Manager::UpdateConfig^(const Config^& config^) { config_ = config; }
echo const Config^& Manager::GetConfig^(^) const { return config_; }
echo }
) > "src\archipelago\core\ap_manager_impl.cpp"
)

:: Add implementation files to CMakeLists.txt
echo Adding implementation files to CMakeLists.txt...
powershell -Command "$content = Get-Content 'src\CMakeLists.txt' -Raw; if ($content -match 'archipelago/core/ap_state.cpp' -and $content -notmatch 'ap_network_impl.cpp') { $content = $content -replace '(archipelago/core/ap_state.cpp)', \"\"$1`n        archipelago/core/ap_network_impl.cpp`n        archipelago/core/ap_manager_impl.cpp\"\"; $content = $content -replace '\"\"\""', '\"' | Set-Content 'src\CMakeLists.txt' -NoNewline }"

echo.
echo ================================================================================
echo All fixes applied!
echo ================================================================================
echo.
echo Next steps:
echo 1. Delete the build_archipelago directory
echo 2. Run your build script
echo.

choice /C YN /M "Delete build directory and rebuild now"
if %errorlevel%==1 (
    if exist "build_archipelago" (
        echo Cleaning build directory...
        rmdir /S /Q "build_archipelago"
    )
    echo.
    if exist "build_archipelago_minimal.bat" (
        call build_archipelago_minimal.bat
    ) else if exist "build_selaco_archipelago.bat" (
        call build_selaco_archipelago.bat
    ) else (
        echo Please run your build script manually.
    )
)

pause