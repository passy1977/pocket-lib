# TO_DO

## Open BUGS

Security issues still open. Numbering matches the internal security review.
Severity: 🔴 critical, 🟠 high.

### 🔴 1. Weak password hashing — single unsalted SHA-512
- **Where:** `pocket-controllers/src/session.cpp` (`login`, `retrieve_data`), `pocket-services/src/crypto.cpp` (`crypto_encode_sha512`)
- **Problem:** the user password is stored/verified as a single, unsalted SHA-512 digest. An attacker who reads the local DB can brute-force it offline at billions of guesses/second and can use precomputed rainbow tables.
- **Fix:** use a real password KDF (Argon2id, scrypt, or PBKDF2) with a random per-user salt.
- **Why it is still open:** changes the stored hash format and the value sent to the server → requires data migration and server-side coordination.

### 🔴 2. AES key derived from the stored password hash
- **Where:** `pocket-controllers/src/session.cpp` (`export_data`/`import_data`, view construction), `pocket-services/src/crypto.cpp` (`aes` ctor)
- **Problem:** the AES key is the first 32 chars of the same SHA-512 hex digest stored in the DB. Whoever reads the DB obtains the data-encryption key directly, so encryption adds no protection against DB compromise. Being hex characters, an AES-256 key carries at most ~128 bits of entropy; keys shorter than 32 bytes are padded with the constant `'$'`.
- **Fix:** derive the data key with a dedicated KDF (separate salt/info from the auth hash).
- **Why it is still open:** changes the ciphertext format in the DB and in exports → requires data migration.

### 🔴 3. Fixed AES-CBC IV
- **Where:** `pocket-controllers/src/session.cpp` (`aes_cbc_iv` from config), `secrets.cmake`
- **Problem:** the IV comes from config and is identical for every encryption (historically a compile-time constant). CBC with a fixed IV is deterministic: equal plaintext fields produce equal ciphertext, leaking patterns.
- **Fix:** generate a random IV per encryption and store it next to the ciphertext.
- **Why it is still open:** changes the ciphertext format → requires data migration.

### 🔴 4. Unauthenticated encryption (CBC without MAC)
- **Where:** `pocket-services/src/crypto.cpp` (`aes::encrypt`/`aes::decrypt`)
- **Problem:** raw AES-CBC is malleable and, with distinct padding-error exceptions, is exposed to padding-oracle attacks when an attacker can submit ciphertext.
- **Fix:** use AES-GCM (or CBC + HMAC in encrypt-then-MAC).
- **Why it is still open:** changes the ciphertext format → requires data migration.

> Items 1–4 are linked: together they mean the current local encryption does **not** effectively protect the data if the DB is compromised. Fixing them requires a small key-derivation redesign (KDF with salt, separate auth/encryption keys, per-record random IV, authenticated mode) plus a migration for existing data.

### 🟠 5. TLS verification can be disabled + cleartext HTTP fallback
- **Where:** `pocket-services/src/network.cpp` (`perform`)
- **Problem:** when built without `POCKET_ENABLE_SSL_VERIFY`, the code explicitly sets `CURLOPT_SSL_VERIFYPEER=0` / `CURLOPT_SSL_VERIFYHOST=0` → trivial MITM. Also, a URL with no scheme is silently prefixed with `http://`, sending even the auth header `cors_header_token` in the clear.
- **Fix:** remove the insecure branch (or limit it to test builds) and reject/upgrade schemeless URLs to HTTPS.
- **Why it is still open:** removing the insecure branch changes behavior for anyone building with `POCKET_ENABLE_SSL_VERIFY=OFF` against a self-signed cert — needs confirmation before changing.

---

## Fixed

Behavior-preserving fixes already applied (no change to the library's observable behavior on valid inputs; API and data formats unchanged).

- **🟠 Use-after-free in async lambdas** — `synchronizer.cpp` captured `string_view::data()` raw pointers into thread-pool tasks; now captures owning `std::string` copies. Same pattern fixed at 5 other `string_view::data()` → `std::string` sites (`synchronizer.cpp`, `session.cpp`, `json.cpp`).
- **🟠 Non-cryptographic RNG** — `crypto_generate_random_string` moved from `mt19937` to OpenSSL `RAND_bytes` with rejection sampling. Same alphabet, length and uniform distribution; only the randomness source changed (now cryptographic).
- **🟡 OOB read in `crypto_base64_encode`** — removed the `data[1]` access on 1-byte input; the removed block never affected the returned output.
- **🟡 Over-read in `aes::decrypt`** — builds `std::string{encrypted}` instead of passing a possibly non-NUL-terminated `string_view::data()`.
- **🟡 Leak / null-deref in `crypto_base64_decode`** — null-checks on `BIO_new`/`BIO_new_mem_buf` and frees BIO + buffer on the error path (previously leaked on every malformed base64 input).

### Tests
- Fixed pre-existing compile/format drift in `SynchronizerServiceTest` (constructor signature) and `SessionTest` (config JSON fields).
- Added 12 crypto tests (regression coverage for the fixes above + full coverage of the previously untested `crypto_encrypt_rsa`). Suite: **102/102 passing**.
