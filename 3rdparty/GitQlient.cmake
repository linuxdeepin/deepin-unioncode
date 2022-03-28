include_directories(
    ${CMAKE_SOURCE_DIR}/3rdparty/GitQlient/QLogger
    ${CMAKE_SOURCE_DIR}/3rdparty/GitQlient/src/jenkins
    ${CMAKE_SOURCE_DIR}/3rdparty/GitQlient/src/QPinnableTabWidget
    ${CMAKE_SOURCE_DIR}/3rdparty/GitQlient/src/git_server
    ${CMAKE_SOURCE_DIR}/3rdparty/GitQlient/src/history
    ${CMAKE_SOURCE_DIR}/3rdparty/GitQlient/src/cache
    ${CMAKE_SOURCE_DIR}/3rdparty/GitQlient/src/git
    ${CMAKE_SOURCE_DIR}/3rdparty/GitQlient/src/diff
    ${CMAKE_SOURCE_DIR}/3rdparty/GitQlient/src/config
    ${CMAKE_SOURCE_DIR}/3rdparty/GitQlient/src/commits
    ${CMAKE_SOURCE_DIR}/3rdparty/GitQlient/src/branches
    ${CMAKE_SOURCE_DIR}/3rdparty/GitQlient/src/big_widgets
    ${CMAKE_SOURCE_DIR}/3rdparty/GitQlient/src/aux_widgets
    )

set(CMAKE_CXX_STANDARD 17)

link_libraries(GitQlient)
