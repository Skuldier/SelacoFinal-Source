#!/usr/bin/env python3
"""
Archipelago Auto-Patcher for Selaco (Fixed Encoding Version)
Handles encoding issues properly
"""

import os
import sys
import shutil
import re
from pathlib import Path

class ArchipelagoPatcher:
    def __init__(self):
        self.root_dir = Path.cwd()
        self.archipelago_dir = self.root_dir / "src" / "archipelago"
        self.dependencies_dir = self.archipelago_dir / "dependencies"
        self.errors = []
        self.fixes_applied = []
        
    def log(self, message, level="INFO"):
        """Print a formatted log message"""
        # Simple prefix without ANSI colors for better compatibility
        prefix = f"[{level}]"
        print(f"{prefix} {message}")
        
    def read_file_safe(self, path):
        """Read a file with proper encoding handling"""
        # Try different encodings
        encodings = ['utf-8', 'utf-8-sig', 'latin-1', 'cp1252']
        
        for encoding in encodings:
            try:
                with open(path, 'r', encoding=encoding) as f:
                    return f.read()
            except UnicodeDecodeError:
                continue
                
        # If all fail, read as binary and decode with errors='ignore'
        with open(path, 'rb') as f:
            return f.read().decode('utf-8', errors='ignore')
            
    def write_file_safe(self, path, content):
        """Write a file with UTF-8 encoding"""
        with open(path, 'w', encoding='utf-8', newline='\n') as f:
            f.write(content)
        
    def check_directory(self):
        """Verify we're in the right directory"""
        if not self.archipelago_dir.exists():
            self.log("Cannot find src/archipelago directory!", "ERROR")
            self.log("Please run this script from the Selaco root directory.", "ERROR")
            return False
        return True
        
    def create_directories(self):
        """Create required directories"""
        self.log("Creating required directories...", "STEP")
        
        dirs_to_create = [
            self.dependencies_dir,
            self.dependencies_dir / "wswrap" / "include",
        ]
        
        for dir_path in dirs_to_create:
            dir_path.mkdir(parents=True, exist_ok=True)
            
        self.fixes_applied.append("Created dependency directories")
        
    def create_archipelago_fixes(self):
        """Create archipelago_fixes.h"""
        self.log("Creating archipelago_fixes.h...", "STEP")
        
        content = '''#pragma once

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
'''
        
        self.write_file_safe(self.dependencies_dir / "archipelago_fixes.h", content)
        self.fixes_applied.append("Created archipelago_fixes.h")
        
    def create_wswrap(self):
        """Create wswrap.hpp stub"""
        self.log("Creating wswrap.hpp...", "STEP")
        
        content = '''#pragma once

// WebSocket wrapper stub for Archipelago
// This file provides a minimal interface for WebSocket functionality

#include <string>
#include <functional>
#include <memory>

namespace wswrap {

class WebSocketClient {
public:
    WebSocketClient() = default;
    virtual ~WebSocketClient() = default;
    
    // Connection callbacks
    std::function<void()> on_open;
    std::function<void(const std::string&)> on_message;
    std::function<void(int, const std::string&)> on_close;
    std::function<void(const std::string&)> on_error;
    
    // Connection methods
    virtual bool connect(const std::string& url) = 0;
    virtual void disconnect() = 0;
    virtual bool send(const std::string& message) = 0;
    virtual bool is_connected() const = 0;
};

// Factory function to create platform-specific WebSocket client
std::unique_ptr<WebSocketClient> create_websocket_client();

} // namespace wswrap
'''
        
        self.write_file_safe(self.dependencies_dir / "wswrap" / "include" / "wswrap.hpp", content)
        self.fixes_applied.append("Created wswrap.hpp")
        
    def create_apclient(self):
        """Create apclient.hpp stub"""
        self.log("Creating apclient.hpp...", "STEP")
        
        content = '''#pragma once
#include "../archipelago_fixes.h"
#include <string>
#include <vector>
#include <functional>
#include <tuple>

class APClient {
public:
    struct NetworkItem {
        int64_t item;
        int64_t location;
        int player;
        int flags;
    };
    
    APClient(const std::string& uuid, const std::string& game, const std::string& uri) {}
    ~APClient() {}
    
    void poll() {}
    void ConnectSlot(const std::string& name, const std::string& password, int items_handling, 
                     const std::vector<std::string>& tags, const std::tuple<int,int,int>& version) {}
    void LocationChecks(const std::vector<int64_t>& locations) {}
    void Say(const std::string& text) {}
    
    // Callbacks
    void set_socket_connected_handler(std::function<void()> f) {}
    void set_socket_disconnected_handler(std::function<void()> f) {}
    void set_slot_connected_handler(std::function<void(const json&)> f) {}
    void set_slot_refused_handler(std::function<void(const std::vector<std::string>&)> f) {}
    void set_items_received_handler(std::function<void(const std::vector<NetworkItem>&)> f) {}
    void set_print_json_handler(std::function<void(const json&)> f) {}
};
'''
        
        self.write_file_safe(self.dependencies_dir / "apclient.hpp", content)
        self.fixes_applied.append("Created apclient.hpp")
        
    def create_apuuid(self):
        """Create apuuid.hpp stub"""
        self.log("Creating apuuid.hpp...", "STEP")
        
        content = '''#pragma once
#include <string>
#include <chrono>

inline std::string ap_get_uuid(const std::string& game_name) {
    // Simple UUID generation for now
    return game_name + "_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
}
'''
        
        self.write_file_safe(self.dependencies_dir / "apuuid.hpp", content)
        self.fixes_applied.append("Created apuuid.hpp")
        
    def fix_includes(self):
        """Fix include statements in source files"""
        self.log("Fixing source file includes...", "STEP")
        
        # Define all the fixes needed
        fixes = {
            "core/ap_manager.cpp": [
                (r'#include <nlohmann/json\.hpp>', '#include "../dependencies/archipelago_fixes.h"'),
            ],
            "core/ap_manager.h": [
                (r'(#include "ap_types\.h")', r'\1\n#include "../dependencies/archipelago_fixes.h"'),
            ],
            "core/ap_protocol.h": [
                (r'#include <nlohmann/json\.hpp>', '#include "../dependencies/archipelago_fixes.h"'),
            ],
            "core/ap_protocol.cpp": [
                (r'(#include "ap_protocol\.h")', r'\1\n#include "../dependencies/archipelago_fixes.h"'),
            ],
            "core/ap_network.cpp": [
                (r'#include "apclient\.hpp"', '#include "../dependencies/apclient.hpp"'),
                (r'#include "apuuid\.hpp"', '#include "../dependencies/apuuid.hpp"'),
                (r'#include <nlohmann/json\.hpp>', '#include "../dependencies/archipelago_fixes.h"'),
                (r'(#include "ap_network\.h")', r'\1\n#include <chrono>\n#include <tuple>'),
            ],
            "core/ap_network.h": [
                (r'(// Forward declaration of APClient)', r'#include "../dependencies/archipelago_fixes.h"\n\n\1'),
            ],
            "core/ap_state.cpp": [
                (r'#include <nlohmann/json\.hpp>', '#include "../dependencies/archipelago_fixes.h"'),
            ],
            "ui/ap_overlay.cpp": [
                (r'common/2d/v_text\.h', 'common/fonts/v_text.h'),
            ],
            "ui/ap_overlay.h": [
                (r'(#include "\.\./core/ap_types\.h")', r'\1\n#include <vector>'),
            ],
        }
        
        for file_path, replacements in fixes.items():
            full_path = self.archipelago_dir / file_path
            if not full_path.exists():
                self.log(f"Warning: {file_path} not found", "WARN")
                continue
                
            content = self.read_file_safe(full_path)
            original_content = content
            
            for pattern, replacement in replacements:
                # Check if the fix is already applied
                if pattern.startswith(r'(#include'):
                    # This is an "add after" pattern
                    if 'archipelago_fixes.h' in content or '<vector>' in content or '<chrono>' in content:
                        continue  # Already fixed
                        
                content = re.sub(pattern, replacement, content)
                
            if content != original_content:
                self.write_file_safe(full_path, content)
                self.log(f"Fixed includes in {file_path}")
                self.fixes_applied.append(f"Fixed {file_path}")
                
    def create_implementation_stubs(self):
        """Create stub implementation files"""
        self.log("Creating implementation stubs...", "STEP")
        
        # Create ap_network_impl.cpp
        network_impl_content = '''#include "ap_network.h"

namespace Archipelago {

// Stub implementations for missing methods
void APNetworkClient::SendLocationScouts(const std::vector<int64_t>& locations) {}
void APNetworkClient::SendBounce(const json& data) {}
void APNetworkClient::SendStatusUpdate(int status) {}
void APNetworkClient::GetData(const std::vector<std::string>& keys) {}
void APNetworkClient::SetData(const std::string& key, const json& value) {}
void APNetworkClient::StopNetworkThread() {}

} // namespace Archipelago
'''
        
        network_impl_path = self.archipelago_dir / "core" / "ap_network_impl.cpp"
        if not network_impl_path.exists():
            self.write_file_safe(network_impl_path, network_impl_content)
            self.fixes_applied.append("Created ap_network_impl.cpp")
            
        # Create ap_manager_impl.cpp
        manager_impl_content = '''#include "ap_manager.h"

namespace Archipelago {

// Stub implementations for missing methods
void Manager::ScoutLocation(int64_t location_id) {}
void Manager::ScoutLocations(const std::vector<int64_t>& location_ids) {}
void Manager::SendDeathLink(const std::string& cause) {}
void Manager::GetData(const std::vector<std::string>& keys) {}
void Manager::SetData(const std::string& key, const json& value) {}
void Manager::SetGoalComplete() {}
void Manager::SetGameComplete() {}
void Manager::SetDeathLinkCallback(DeathLinkCallback callback) {}
std::vector<Message> Manager::GetAllMessages() { return {}; }
const NetworkPlayer* Manager::GetPlayer(int slot) const { return nullptr; }
const NetworkPlayer* Manager::GetLocalPlayer() const { return nullptr; }
void Manager::UpdateConfig(const Config& config) { config_ = config; }
const Config& Manager::GetConfig() const { return config_; }

} // namespace Archipelago
'''
        
        manager_impl_path = self.archipelago_dir / "core" / "ap_manager_impl.cpp"
        if not manager_impl_path.exists():
            self.write_file_safe(manager_impl_path, manager_impl_content)
            self.fixes_applied.append("Created ap_manager_impl.cpp")
            
    def fix_cmakelists(self):
        """Fix CMakeLists.txt"""
        self.log("Updating CMakeLists.txt...", "STEP")
        
        cmake_path = self.root_dir / "src" / "CMakeLists.txt"
        if not cmake_path.exists():
            self.log("CMakeLists.txt not found!", "ERROR")
            return
            
        # Backup
        backup_path = cmake_path.with_suffix('.txt.backup')
        if not backup_path.exists():
            shutil.copy2(cmake_path, backup_path)
            
        content = self.read_file_safe(cmake_path)
        original_content = content
        
        # Add namespace definition
        if 'NLOHMANN_JSON_NAMESPACE_NO_VERSION' not in content:
            content = re.sub(
                r'(add_definitions\(-DASIO_STANDALONE=1\))',
                r'\1\n    \n    # FIX: Disable JSON versioned namespace to avoid conflicts\n    add_definitions(-DNLOHMANN_JSON_NAMESPACE_NO_VERSION=1)',
                content
            )
            
        # Add implementation files
        if 'ap_network_impl.cpp' not in content and 'archipelago/core/ap_state.cpp' in content:
            content = re.sub(
                r'(archipelago/core/ap_state\.cpp)',
                r'\1\n        archipelago/core/ap_network_impl.cpp\n        archipelago/core/ap_manager_impl.cpp',
                content
            )
            
        if content != original_content:
            self.write_file_safe(cmake_path, content)
            self.log("Updated CMakeLists.txt")
            self.fixes_applied.append("Updated CMakeLists.txt")
            
    def verify_fixes(self):
        """Verify all fixes were applied"""
        self.log("Verifying fixes...", "STEP")
        
        required_files = [
            self.dependencies_dir / "archipelago_fixes.h",
            self.dependencies_dir / "wswrap" / "include" / "wswrap.hpp",
            self.dependencies_dir / "apclient.hpp",
            self.dependencies_dir / "apuuid.hpp",
            self.archipelago_dir / "core" / "ap_network_impl.cpp",
            self.archipelago_dir / "core" / "ap_manager_impl.cpp",
        ]
        
        all_good = True
        for file_path in required_files:
            if file_path.exists():
                self.log(f"OK: {file_path.name} exists", "INFO")
            else:
                self.log(f"FAIL: {file_path.name} missing", "ERROR")
                all_good = False
                
        return all_good
        
    def clean_build_directory(self):
        """Clean the build directory"""
        build_dir = self.root_dir / "build_archipelago"
        if build_dir.exists():
            self.log("Cleaning build directory...", "STEP")
            shutil.rmtree(build_dir)
            self.log("Build directory cleaned")
            
    def run(self):
        """Run the complete patching process"""
        print("=" * 80)
        print("Archipelago Auto-Patcher for Selaco (Fixed Encoding Version)")
        print("=" * 80)
        print()
        
        if not self.check_directory():
            return False
            
        try:
            self.create_directories()
            self.create_archipelago_fixes()
            self.create_wswrap()
            self.create_apclient()
            self.create_apuuid()
            self.fix_includes()
            self.create_implementation_stubs()
            self.fix_cmakelists()
            
            print()
            print("=" * 80)
            self.log("All fixes applied successfully!", "INFO")
            print("=" * 80)
            print()
            
            print("Fixes applied:")
            for fix in self.fixes_applied:
                print(f"  - {fix}")
                
            print()
            
            if not self.verify_fixes():
                self.log("Some files are missing. Please check the errors above.", "WARN")
                return False
                
            # Ask to rebuild
            response = input("\nDelete build directory and rebuild now? (y/n): ")
            if response.lower() == 'y':
                self.clean_build_directory()
                
                # Try to run build script
                build_scripts = [
                    "build_archipelago_minimal.bat",
                    "build_selaco_archipelago.bat",
                    "Build-SelacoArchipelago.ps1"
                ]
                
                for script in build_scripts:
                    if (self.root_dir / script).exists():
                        self.log(f"Running {script}...", "INFO")
                        os.system(script)
                        break
                else:
                    self.log("No build script found. Please run your build script manually.", "WARN")
                    
            return True
            
        except Exception as e:
            self.log(f"Error: {str(e)}", "ERROR")
            import traceback
            traceback.print_exc()
            return False


if __name__ == "__main__":
    patcher = ArchipelagoPatcher()
    success = patcher.run()
    
    if not success:
        sys.exit(1)
        
    print("\nPress Enter to exit...")
    input()