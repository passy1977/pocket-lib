#include <iostream>
#include <pocket/globals.hpp>
#include <pocket-services/crypto.hpp>

int main() {
    std::cout << "Pocket Library Example Usage\n";
    std::cout << "===========================\n\n";
    
    // Mostra le configurazioni attive
    std::cout << "Active configurations:\n";
    
#ifdef POCKET_DISABLE_LOCK
    std::cout << "  ✓ POCKET_DISABLE_LOCK is enabled\n";
#else
    std::cout << "  ✗ POCKET_DISABLE_LOCK is disabled\n";
#endif

#ifdef POCKET_DISABLE_DB_LOCK
    std::cout << "  ✓ POCKET_DISABLE_DB_LOCK is enabled\n";
#else
    std::cout << "  ✗ POCKET_DISABLE_DB_LOCK is disabled\n";
#endif

#ifdef POCKET_ENABLE_LOG
    std::cout << "  ✓ POCKET_ENABLE_LOG is enabled\n";
#else
    std::cout << "  ✗ POCKET_ENABLE_LOG is disabled\n";
#endif

#ifdef POCKET_ENABLE_AES
    std::cout << "  ✓ POCKET_ENABLE_AES is enabled\n";
#else
    std::cout << "  ✗ POCKET_ENABLE_AES is disabled\n";
#endif

#ifdef POCKET_ENABLE_TEST
    std::cout << "  ✓ POCKET_ENABLE_TEST is enabled\n";
#else
    std::cout << "  ✗ POCKET_ENABLE_TEST is disabled\n";
#endif

#ifdef POCKET_MAX_BUFFER_RESPONSE_SIZE
    std::cout << "  📦 POCKET_MAX_BUFFER_RESPONSE_SIZE = " << POCKET_MAX_BUFFER_RESPONSE_SIZE << " bytes\n";
#endif

#ifdef POCKET_FORCE_TIMESTAMP_LAST_UPDATE
    std::cout << "  🕒 POCKET_FORCE_TIMESTAMP_LAST_UPDATE = " << POCKET_FORCE_TIMESTAMP_LAST_UPDATE << "\n";
#endif

    std::cout << "\nPocket library is ready to use!\n";
    std::cout << "All headers and dependencies are automatically available.\n";
    
    return 0;
}