# README

[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/passy1977/pocket-lib)

*🇬🇧 [English](#english) | 🇮🇹 [Italiano](#italiano)*

---

## English

### Table of Contents
1. [Overview](#overview)
2. [Installation on Debian 12](#installation-on-debian-12)
3. [Configuration Options](#configuration-options)
4. [Usage Methods](#usage-methods)
5. [Building and Testing](#building-and-testing)
6. [Examples](#examples)
7. [Technical Notes](#technical-notes)

### Overview

Pocket-lib is a secure library for credential and password storage, providing a comprehensive system for managing, encrypting, and synchronizing sensitive user data. The library implements a secure wallet system that allows users to store credentials in a hierarchical structure of groups and fields, with robust encryption, local persistence, and remote synchronization capabilities.

**Key Features:**
- 🔐 **Secure Encryption**: AES encryption support for sensitive data
- 🗄️ **Database Storage**: SQLite3-based local persistence
- 🌐 **Network Synchronization**: CURL-based remote synchronization
- 🏗️ **Modular Architecture**: Component-based design with clear separation of concerns
- ⚙️ **Configurable**: Extensive configuration options for different use cases
- 🔧 **CMake Integration**: Modern CMake support with automatic dependency management

### Installation on Debian 12

#### Prerequisites

Before installing Pocket Library, you need to install the required system dependencies:

```bash
# Update package repositories
sudo apt update

# Install build tools
sudo apt install -y build-essential cmake git pkg-config

# Install required libraries
sudo apt install -y \
    libcurl4-openssl-dev \
    libssl-dev \
    libsqlite3-dev

# Optional: Install additional development tools
sudo apt install -y \
    clang \
    clang-format \
    gdb \
    valgrind
```

#### Method 1: Clone and Build

```bash
# Clone the repository
git clone https://github.com/passy1977/pocket-lib.git
cd pocket-lib

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DPOCKET_ENABLE_TEST=ON

# Build the library
make -j$(nproc)

# Run tests (optional)
make test
```

#### Method 2: Using as Git Submodule

```bash
# In your project directory
git submodule add https://github.com/passy1977/pocket-lib.git external/pocket-lib
git submodule update --init --recursive
```

### Configuration Options

#### Boolean Options
- `POCKET_DISABLE_LOCK` (default: ON) - Disable locking mechanisms
- `POCKET_DISABLE_DB_LOCK` (default: ON) - Disable database locking
- `POCKET_ENABLE_LOG` (default: ON) - Enable logging functionality
- `POCKET_ENABLE_AES` (default: ON) - Enable AES encryption
- `POCKET_ENABLE_TEST` (default: OFF) - Enable test compilation

#### Advanced Options
- `POCKET_MAX_BUFFER_RESPONSE_SIZE` (default: 10485760) - Maximum response buffer size
- `POCKET_FORCE_TIMESTAMP_LAST_UPDATE` (default: 0) - Force specific timestamp (0 = disabled)

### Usage Methods

#### Method 1: Submodule with Custom Configuration

```cmake
cmake_minimum_required(VERSION 3.25)
project(MyProject)

set(CMAKE_CXX_STANDARD 20)

# Configure Pocket before including it
set(POCKET_DISABLE_LOCK OFF)  # Enable locking
set(POCKET_ENABLE_LOG OFF)    # Disable logging
set(POCKET_MAX_BUFFER_RESPONSE_SIZE 5242880)  # 5MB instead of default 10MB

# Add Pocket as subdirectory
add_subdirectory(external/pocket-lib)

add_executable(myapp main.cpp)

# Link Pocket - automatically imports all dependencies and include paths
target_link_libraries(myapp pocket)
```

#### Method 2: FetchContent with Configuration

```cmake
cmake_minimum_required(VERSION 3.25)
project(MyProject)

set(CMAKE_CXX_STANDARD 20)

# Configure Pocket before download
set(POCKET_DISABLE_LOCK OFF)
set(POCKET_ENABLE_AES OFF)  # Disable AES if not needed
set(POCKET_MAX_BUFFER_RESPONSE_SIZE 20971520)  # 20MB for large responses

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

#### Method 3: Command Line Configuration

```bash
mkdir build && cd build
cmake .. \
  -DPOCKET_DISABLE_LOCK=OFF \
  -DPOCKET_ENABLE_LOG=OFF \
  -DPOCKET_MAX_BUFFER_RESPONSE_SIZE=20971520 \
  -DPOCKET_FORCE_TIMESTAMP_LAST_UPDATE=1739741159
make -j$(nproc)
```

### Building and Testing

#### Enable and Run Tests

```bash
# Configure with tests enabled
cmake .. -DPOCKET_ENABLE_TEST=ON
make -j$(nproc)

# Run tests
make test
# or directly
./pocket-test
```

### Examples

#### Basic Usage Example

```cpp
#include <pocket/globals.hpp>
#include <pocket-controllers/session.hpp>
#include <pocket-daos/dao.hpp>
#include <pocket-services/crypto.hpp>

int main() {
    // Initialize Pocket library
    // Your application code here
    return 0;
}
```

#### Common Configurations

**Production Configuration (Performance Optimized):**
```cmake
set(POCKET_DISABLE_LOCK ON)
set(POCKET_DISABLE_DB_LOCK ON)
set(POCKET_ENABLE_LOG OFF)          # Disable logging for performance
set(POCKET_ENABLE_AES ON)
set(POCKET_MAX_BUFFER_RESPONSE_SIZE 10485760)
```

**Debug Configuration:**
```cmake
set(POCKET_DISABLE_LOCK OFF)        # Enable locking for safety
set(POCKET_DISABLE_DB_LOCK OFF)
set(POCKET_ENABLE_LOG ON)           # Enable logging for debugging
set(POCKET_ENABLE_AES ON)
set(POCKET_MAX_BUFFER_RESPONSE_SIZE 1048576)  # Smaller buffer for testing
```

### Technical Notes

- The library uses generator expressions to distinguish between BUILD_INTERFACE and INSTALL_INTERFACE
- External dependencies are linked as PRIVATE but their functionality is available through Pocket's API
- The alias target `Pocket::pocket` is available for modern namespace compatibility
- System installation is currently disabled to avoid conflicts with automatically downloaded dependencies

---

## Italiano

### Indice
1. [Panoramica](#panoramica)
2. [Installazione su Debian 12](#installazione-su-debian-12)
3. [Opzioni di Configurazione](#opzioni-di-configurazione)
4. [Metodi di Utilizzo](#metodi-di-utilizzo)
5. [Compilazione e Test](#compilazione-e-test)
6. [Esempi](#esempi)
7. [Note Tecniche](#note-tecniche)

### Panoramica

Pocket-lib è una libreria sicura per l'archiviazione di credenziali e password, che fornisce un sistema completo per gestire, crittografare e sincronizzare dati sensibili degli utenti. La libreria implementa un sistema di portafoglio sicuro che consente agli utenti di memorizzare le credenziali in una struttura gerarchica di gruppi e campi, con crittografia robusta, persistenza locale e capacità di sincronizzazione remota.

**Caratteristiche Principali:**
- 🔐 **Crittografia Sicura**: Supporto per crittografia AES per dati sensibili
- 🗄️ **Archiviazione Database**: Persistenza locale basata su SQLite3
- 🌐 **Sincronizzazione di Rete**: Sincronizzazione remota basata su CURL
- 🏗️ **Architettura Modulare**: Design basato su componenti con chiara separazione delle responsabilità
- ⚙️ **Configurabile**: Opzioni di configurazione estese per diversi casi d'uso
- 🔧 **Integrazione CMake**: Supporto CMake moderno con gestione automatica delle dipendenze

### Installazione su Debian 12

#### Prerequisiti

Prima di installare Pocket Library, devi installare le dipendenze di sistema richieste:

```bash
# Aggiorna i repository dei pacchetti
sudo apt update

# Installa gli strumenti di build
sudo apt install -y build-essential cmake git pkg-config

# Installa le librerie richieste
sudo apt install -y \
    libcurl4-openssl-dev \
    libssl-dev \
    libsqlite3-dev

# Opzionale: Installa strumenti di sviluppo aggiuntivi
sudo apt install -y \
    clang \
    clang-format \
    gdb \
    valgrind
```

#### Metodo 1: Clona e Compila

```bash
# Clona il repository
git clone https://github.com/passy1977/pocket-lib.git
cd pocket-lib

# Crea la directory di build
mkdir build && cd build

# Configura con CMake
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DPOCKET_ENABLE_TEST=ON

# Compila la libreria
make -j$(nproc)

# Esegui i test (opzionale)
make test
```

#### Metodo 2: Utilizzo come Git Submodule

```bash
# Nella directory del tuo progetto
git submodule add https://github.com/passy1977/pocket-lib.git external/pocket-lib
git submodule update --init --recursive
```

### Opzioni di Configurazione

#### Opzioni Boolean
- `POCKET_DISABLE_LOCK` (default: ON) - Disabilita i meccanismi di locking
- `POCKET_DISABLE_DB_LOCK` (default: ON) - Disabilita il locking del database
- `POCKET_ENABLE_LOG` (default: ON) - Abilita la funzionalità di logging
- `POCKET_ENABLE_AES` (default: ON) - Abilita la crittografia AES
- `POCKET_ENABLE_TEST` (default: OFF) - Abilita la compilazione dei test

#### Opzioni Avanzate
- `POCKET_MAX_BUFFER_RESPONSE_SIZE` (default: 10485760) - Dimensione massima del buffer di risposta
- `POCKET_FORCE_TIMESTAMP_LAST_UPDATE` (default: 0) - Forza un timestamp specifico (0 = disabilitato)

### Metodi di Utilizzo

#### Metodo 1: Submodule con Configurazione Personalizzata

```cmake
cmake_minimum_required(VERSION 3.25)
project(MyProject)

set(CMAKE_CXX_STANDARD 20)

# Configura Pocket prima di includerlo
set(POCKET_DISABLE_LOCK OFF)  # Abilita il locking
set(POCKET_ENABLE_LOG OFF)    # Disabilita il logging
set(POCKET_MAX_BUFFER_RESPONSE_SIZE 5242880)  # 5MB invece dei default 10MB

# Aggiungi Pocket come subdirectory
add_subdirectory(external/pocket-lib)

add_executable(myapp main.cpp)

# Linka Pocket - importa automaticamente tutte le dipendenze e i percorsi include
target_link_libraries(myapp pocket)
```

#### Metodo 2: FetchContent con Configurazione

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

#### Metodo 3: Configurazione da Linea di Comando

```bash
mkdir build && cd build
cmake .. \
  -DPOCKET_DISABLE_LOCK=OFF \
  -DPOCKET_ENABLE_LOG=OFF \
  -DPOCKET_MAX_BUFFER_RESPONSE_SIZE=20971520 \
  -DPOCKET_FORCE_TIMESTAMP_LAST_UPDATE=1739741159
make -j$(nproc)
```

### Compilazione e Test

#### Abilita ed Esegui i Test

```bash
# Configura con i test abilitati
cmake .. -DPOCKET_ENABLE_TEST=ON
make -j$(nproc)

# Esegui i test
make test
# oppure direttamente
./pocket-test
```

### Esempi

#### Esempio di Utilizzo Base

```cpp
#include <pocket/globals.hpp>
#include <pocket-controllers/session.hpp>
#include <pocket-daos/dao.hpp>
#include <pocket-services/crypto.hpp>

int main() {
    // Inizializza la libreria Pocket
    // Il tuo codice applicativo qui
    return 0;
}
```

#### Configurazioni Comuni

**Configurazione per Produzione (Ottimizzata per Performance):**
```cmake
set(POCKET_DISABLE_LOCK ON)
set(POCKET_DISABLE_DB_LOCK ON)
set(POCKET_ENABLE_LOG OFF)          # Disabilita logging per performance
set(POCKET_ENABLE_AES ON)
set(POCKET_MAX_BUFFER_RESPONSE_SIZE 10485760)
```

**Configurazione per Debug:**
```cmake
set(POCKET_DISABLE_LOCK OFF)        # Abilita locking per sicurezza
set(POCKET_DISABLE_DB_LOCK OFF)
set(POCKET_ENABLE_LOG ON)           # Abilita logging per debug
set(POCKET_ENABLE_AES ON)
set(POCKET_MAX_BUFFER_RESPONSE_SIZE 1048576)  # Buffer più piccolo per test
```

**Configurazione per Sviluppo con Test:**
```cmake
set(POCKET_DISABLE_LOCK OFF)        # Abilita locking per sicurezza
set(POCKET_DISABLE_DB_LOCK OFF)
set(POCKET_ENABLE_LOG ON)           # Abilita logging per debug
set(POCKET_ENABLE_AES ON)
set(POCKET_ENABLE_TEST ON)          # Abilita i test per sviluppo
set(POCKET_MAX_BUFFER_RESPONSE_SIZE 1048576)  # Buffer più piccolo per test
```

**Configurazione per CI/CD con Test:**
```cmake
set(POCKET_DISABLE_LOCK ON)
set(POCKET_DISABLE_DB_LOCK ON)
set(POCKET_ENABLE_LOG ON)
set(POCKET_ENABLE_AES ON)
set(POCKET_ENABLE_TEST ON)          # Abilita i test per CI
set(POCKET_FORCE_TIMESTAMP_LAST_UPDATE 1739741159)  # Timestamp fisso per test riproducibili
```

### Include Automatici

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

### Dipendenze Automatiche

Tutti i metodi sopra gestiscono automaticamente le seguenti dipendenze:
- **CURL** - per le comunicazioni di rete
- **OpenSSL** - per la crittografia
- **SQLite3** - per il database
- **nlohmann/json** - per il parsing JSON (scaricato automaticamente)
- **tinyxml2** - per il parsing XML (scaricato automaticamente)
- **Thread Pool** - incluso nella libreria

### Note Tecniche

- La libreria utilizza generator expressions per distinguere tra BUILD_INTERFACE e INSTALL_INTERFACE
- Le dipendenze esterne sono linkate come PRIVATE ma le loro funzionalità sono disponibili attraverso l'API di Pocket
- Il target alias `Pocket::pocket` è disponibile per compatibilità con namespace moderni
- L'installazione di sistema è attualmente disabilitata per evitare conflitti con le dipendenze scaricate automaticamente

---

## License

This project is licensed under the terms specified in the LICENSE file.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Support

For questions and support, please open an issue on the GitHub repository.
