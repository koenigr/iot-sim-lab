# Glossary

| Term | Meaning | Evidence in repo |
|---|---|---|
| CMake | Build system, generates build files from `CMakeLists.txt` | `CMakeLists.txt` |
| CI/CD | Build and tests run automatically on every push | `.github/workflows/ci.yml` |
| Unit testing | Automatically verifying small units of code | `sensor-node/tests/` |
| Static analysis | Checking code for defects without running it (clang-tidy) | `.clang-tidy`, `.github/workflows/ci.yml` |
| Code style | Automatic, consistent formatting (clang-format) | `.clang-format`, `scripts/format.sh` |
| Containerization | Running programs isolated in containers | `deploy/docker-compose.yml` |