# Copyright 2018 Pawel Bylica.
# Licensed under the Apache License, Version 2.0. See the LICENSE file.

# Read the git commit hash from a file. The gitinfo is suppose to update the
# file only if the hash changes.
file(READ ${BINARY_DIR}/git_commit_hash.txt GIT_COMMIT_HASH)

string(TIMESTAMP TIMESTAMP)
configure_file(${CMAKE_CURRENT_LIST_DIR}/buildinfo.c.in ${BINARY_DIR}/${NAME}.c)

message(
    "       Project Version: ${PROJECT_VERSION}\n"
    "       Git Commit Hash: ${GIT_COMMIT_HASH}\n"
    "       Timestamp:       ${TIMESTAMP}"
)


