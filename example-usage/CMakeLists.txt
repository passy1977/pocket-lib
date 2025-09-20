cmake_minimum_required(VERSION 3.25)
project(PocketExampleUsage)

set(CMAKE_CXX_STANDARD 20)

# Example: Configure Pocket Library before including it
# Queste opzioni possono essere configurate per adattare 
# il comportamento della libreria alle esigenze specifiche

# Configurazione per produzione ottimizzata
set(POCKET_DISABLE_LOCK ON)         # Disabilita locking per performance
set(POCKET_DISABLE_DB_LOCK ON)      # Disabilita DB locking
set(POCKET_ENABLE_LOG OFF)          # Disabilita logging per performance
set(POCKET_ENABLE_AES ON)           # Mantieni crittografia
# set(POCKET_ENABLE_TEST OFF)       # Disabilita test per produzione (commentato per permettere override via CLI)
set(POCKET_MAX_BUFFER_RESPONSE_SIZE 20971520)  # 20MB per risposte grandi

# Per abilitare i test durante lo sviluppo, decommentare la linea seguente:
# set(POCKET_ENABLE_TEST ON)

# Se hai bisogno di timestamp fisso per test
# set(POCKET_FORCE_TIMESTAMP_LAST_UPDATE 1739741159)

# Includi la libreria Pocket dal parent directory
add_subdirectory(.. pocket-lib)

# Crea un'applicazione di esempio
add_executable(example-app main.cpp)

# Linka la libreria Pocket - tutte le configurazioni vengono automaticamente applicate
target_link_libraries(example-app pocket)

# Le seguenti definizioni sono ora automaticamente disponibili:
# - POCKET_DISABLE_LOCK (se abilitato)
# - POCKET_DISABLE_DB_LOCK (se abilitato) 
# - POCKET_ENABLE_LOG (se abilitato)
# - POCKET_ENABLE_AES (se abilitato)
# - POCKET_MAX_BUFFER_RESPONSE_SIZE=20971520
# - POCKET_FORCE_TIMESTAMP_LAST_UPDATE (se impostato)

message(STATUS "Example app configured with:")
message(STATUS "  Pocket linked with all configurations applied automatically")
message(STATUS "  All Pocket headers available via #include <pocket/...>")
message(STATUS "  All dependencies (CURL, OpenSSL, SQLite3, etc.) linked automatically")