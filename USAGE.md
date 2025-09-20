# Come utilizzare Pocket Library in altri progetti

La libreria Pocket è configurata per propagare automaticamente tutti gli include directories necessari e le dipendenze quando viene utilizzata in altri progetti. Inoltre, può essere facilmente configurata tramite opzioni CMake.

## Configurazione della libreria

La libreria è stata ottimizzata per:
- ✅ **Include automatici**: Tutti gli header directories vengono automaticamente aggiunti ai progetti che linkano Pocket
- ✅ **Dipendenze gestite**: Le dipendenze esterne (CURL, OpenSSL, SQLite3, nlohmann_json, tinyxml2) sono gestite automaticamente
- ✅ **Compatibilità moderna**: Utilizza le best practice di CMake con target properties e generator expressions
- ✅ **Configurabile**: Tutte le opzioni di compilazione possono essere personalizzate dal progetto chiamante

## Opzioni di configurazione

La libreria Pocket supporta le seguenti opzioni di configurazione:

### Opzioni Boolean
- `POCKET_DISABLE_LOCK` (default: ON) - Disabilita i meccanismi di locking
- `POCKET_DISABLE_DB_LOCK` (default: ON) - Disabilita il locking del database
- `POCKET_ENABLE_LOG` (default: ON) - Abilita il logging
- `POCKET_ENABLE_AES` (default: ON) - Abilita la crittografia AES
- `POCKET_ENABLE_TEST` (default: OFF) - Abilita la compilazione dei test

### Opzioni avanzate
- `POCKET_MAX_BUFFER_RESPONSE_SIZE` (default: 10485760) - Dimensione massima del buffer di risposta
- `POCKET_FORCE_TIMESTAMP_LAST_UPDATE` (default: 0) - Forza un timestamp specifico (0 = disabilitato)

## Metodo 1: Utilizzo come submodule con configurazione personalizzata

Aggiungi Pocket come submodule del tuo progetto:

```bash
git submodule add https://github.com/passy1977/pocket-lib.git external/pocket-lib
```

Nel tuo `CMakeLists.txt` con configurazione personalizzata:

```cmake
cmake_minimum_required(VERSION 3.25)
project(MyProject)

set(CMAKE_CXX_STANDARD 20)

# Configura Pocket prima di includerlo
set(POCKET_DISABLE_LOCK OFF)  # Abilita il locking
set(POCKET_ENABLE_LOG OFF)    # Disabilita il logging
set(POCKET_MAX_BUFFER_RESPONSE_SIZE 5242880)  # 5MB invece dei default 10MB
set(POCKET_FORCE_TIMESTAMP_LAST_UPDATE 1739741159)  # Timestamp fisso

# Aggiungi Pocket come subdirectory
add_subdirectory(external/pocket-lib)

add_executable(myapp main.cpp)

# Linka Pocket - questo importerà automaticamente tutte le dipendenze e include paths
# Le configurazioni sopra vengono automaticamente applicate
target_link_libraries(myapp pocket)
```

## Metodo 2: Utilizzo con FetchContent e configurazione

Nel tuo `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.25)
project(MyProject)

set(CMAKE_CXX_STANDARD 20)

# Configura Pocket prima del download
set(POCKET_DISABLE_LOCK OFF)
set(POCKET_ENABLE_AES OFF)  # Disabilita AES se non necessario
set(POCKET_MAX_BUFFER_RESPONSE_SIZE 20971520)  # 20MB per risposte grandi

include(FetchContent)

FetchContent_Declare(
    pocket
    GIT_REPOSITORY https://github.com/passy1977/pocket-lib.git
    GIT_TAG main
)

FetchContent_MakeAvailable(pocket)

add_executable(myapp main.cpp)
target_link_libraries(myapp pocket)
```

## Metodo 3: Configurazione via linea di comando

Puoi anche configurare le opzioni tramite la linea di comando di CMake:

```bash
mkdir build && cd build
cmake .. \
  -DPOCKET_DISABLE_LOCK=OFF \
  -DPOCKET_ENABLE_LOG=OFF \
  -DPOCKET_MAX_BUFFER_RESPONSE_SIZE=20971520 \
  -DPOCKET_FORCE_TIMESTAMP_LAST_UPDATE=1739741159
make -j$(nproc)
```

## Configurazioni comuni

### Configurazione per produzione (performance ottimizzata)
```cmake
set(POCKET_DISABLE_LOCK ON)
set(POCKET_DISABLE_DB_LOCK ON)
set(POCKET_ENABLE_LOG OFF)          # Disabilita logging per performance
set(POCKET_ENABLE_AES ON)
set(POCKET_MAX_BUFFER_RESPONSE_SIZE 10485760)
```

### Configurazione per debug
```cmake
set(POCKET_DISABLE_LOCK OFF)        # Abilita locking per sicurezza
set(POCKET_DISABLE_DB_LOCK OFF)
set(POCKET_ENABLE_LOG ON)           # Abilita logging per debug
set(POCKET_ENABLE_AES ON)
set(POCKET_MAX_BUFFER_RESPONSE_SIZE 1048576)  # Buffer più piccolo per test
```

### Configurazione per sviluppo con test
```cmake
set(POCKET_DISABLE_LOCK OFF)        # Abilita locking per sicurezza
set(POCKET_DISABLE_DB_LOCK OFF)
set(POCKET_ENABLE_LOG ON)           # Abilita logging per debug
set(POCKET_ENABLE_AES ON)
set(POCKET_ENABLE_TEST ON)          # Abilita i test per sviluppo
set(POCKET_MAX_BUFFER_RESPONSE_SIZE 1048576)  # Buffer più piccolo per test
```

### Configurazione per CI/CD con test
```cmake
set(POCKET_DISABLE_LOCK ON)
set(POCKET_DISABLE_DB_LOCK ON)
set(POCKET_ENABLE_LOG ON)
set(POCKET_ENABLE_AES ON)
set(POCKET_ENABLE_TEST ON)          # Abilita i test per CI
set(POCKET_FORCE_TIMESTAMP_LAST_UPDATE 1739741159)  # Timestamp fisso per test riproducibili
```

## Compilazione ed esecuzione dei test

Per abilitare i test, imposta `POCKET_ENABLE_TEST=ON`:

```bash
# Metodo 1: Via CMake command line
cmake .. -DPOCKET_ENABLE_TEST=ON
make -j$(nproc)

# Esegui i test
make test
# oppure direttamente
./pocket-test

# Metodo 2: Nel CMakeLists.txt del progetto chiamante
set(POCKET_ENABLE_TEST ON)
add_subdirectory(external/pocket-lib)
```

**Nota**: Quando `POCKET_ENABLE_TEST=OFF` (default), GoogleTest non viene scaricato e i test non vengono compilati, riducendo i tempi di build per progetti che non necessitano di test.

## Metodo 4: Copia diretta del codice

Se preferisci copiare direttamente il codice:

1. Copia tutte le directory `pocket-*`, `inc`, `src`, e `thread-pool` nel tuo progetto
2. Nel tuo `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.25)
project(MyProject)

set(CMAKE_CXX_STANDARD 20)

# Configura Pocket
set(POCKET_ENABLE_LOG OFF)
set(POCKET_MAX_BUFFER_RESPONSE_SIZE 5242880)

# Aggiungi le dipendenze necessarie
find_package(PkgConfig REQUIRED)
pkg_search_module(CURL REQUIRED libcurl)
pkg_search_module(OPENSSL REQUIRED openssl)
pkg_search_module(SQLITE3 REQUIRED sqlite3)

# Includi pocket come subdirectory
add_subdirectory(pocket-lib)  # o il percorso dove hai copiato il codice

add_executable(myapp main.cpp)
target_link_libraries(myapp pocket)
```

## Include automatici

Con qualsiasi dei metodi sopra, avrai automaticamente accesso a tutti gli header di Pocket:

```cpp
#include <pocket/globals.hpp>
#include <pocket/tree.hpp>
#include <pocket-controllers/session.hpp>
#include <pocket-daos/dao.hpp>
#include <pocket-iface/pod.hpp>
#include <pocket-pods/response.hpp>
#include <pocket-services/crypto.hpp>
#include <pocket-views/view.hpp>
// E tutti gli altri header...
```

## Dipendenze automatiche

Tutti i metodi sopra gestiscono automaticamente le seguenti dipendenze:
- **CURL** - per le comunicazioni di rete
- **OpenSSL** - per la crittografia
- **SQLite3** - per il database
- **nlohmann/json** - per JSON parsing (scaricato automaticamente)
- **tinyxml2** - per XML parsing (scaricato automaticamente)
- **Thread Pool** - incluso nella libreria

**Importante**: Le dipendenze di sistema (CURL, OpenSSL, SQLite3) devono essere installate sul sistema. Su Ubuntu/Debian:
```bash
sudo apt-get install libcurl4-openssl-dev libssl-dev libsqlite3-dev
```

## Note tecniche

- La libreria utilizza generator expressions per distinguere tra BUILD_INTERFACE e INSTALL_INTERFACE
- Le dipendenze esterne sono linkate come PRIVATE ma le loro funzionalità sono disponibili attraverso l'API di Pocket
- Il target alias `Pocket::pocket` è disponibile per compatibilità con namespace moderni
- L'installazione di sistema è attualmente disabilitata per evitare conflitti con le dipendenze scaricate automaticamente