# Pocket Library - Esempio di Configurazione

Questo esempio dimostra come configurare la libreria Pocket tramite opzioni CMake.

## Come compilare ed eseguire

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
./example-app
```

## Configurazione Personalizzata

Per testare configurazioni diverse, puoi modificare le opzioni nel `CMakeLists.txt` o passarle via linea di comando:

```bash
# Esempio con configurazioni specifiche
cmake .. \
  -DPOCKET_DISABLE_LOCK=OFF \
  -DPOCKET_ENABLE_LOG=ON \
  -DPOCKET_MAX_BUFFER_RESPONSE_SIZE=1048576 \
  -DPOCKET_FORCE_TIMESTAMP_LAST_UPDATE=1739741159

make -j$(nproc)
./example-app
```

## Opzioni Disponibili

- `POCKET_DISABLE_LOCK` (ON/OFF) - Disabilita i meccanismi di locking
- `POCKET_DISABLE_DB_LOCK` (ON/OFF) - Disabilita il locking del database
- `POCKET_ENABLE_LOG` (ON/OFF) - Abilita il logging
- `POCKET_ENABLE_AES` (ON/OFF) - Abilita la crittografia AES
- `POCKET_ENABLE_TEST` (ON/OFF) - Abilita la compilazione dei test
- `POCKET_MAX_BUFFER_RESPONSE_SIZE` (numero) - Dimensione massima del buffer
- `POCKET_FORCE_TIMESTAMP_LAST_UPDATE` (numero) - Timestamp fisso (0 = disabilitato)

Tutte le opzioni configurate vengono automaticamente propagate ai progetti che utilizzano la libreria Pocket.

## Testare con i test abilitati

Per abilitare e eseguire i test:

```bash
# Configurazione con test abilitati
cmake .. \
  -DPOCKET_ENABLE_TEST=ON \
  -DPOCKET_ENABLE_LOG=ON

make -j$(nproc)

# Se i test sono abilitati nella libreria Pocket, il target pocket-test sarà disponibile
cd pocket-lib && make test
```