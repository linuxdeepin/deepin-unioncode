include_directories(
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-GitQlient/QLogger
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-GitQlient/src/jenkins
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-GitQlient/src/QPinnableTabWidget
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-GitQlient/src/git_server
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-GitQlient/src/history
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-GitQlient/src/cache
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-GitQlient/src/git
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-GitQlient/src/diff
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-GitQlient/src/config
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-GitQlient/src/commits
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-GitQlient/src/branches
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-GitQlient/src/big_widgets
    ${CMAKE_SOURCE_DIR}/3rdparty/unioncode-GitQlient/src/aux_widgets
    )

set(CMAKE_CXX_STANDARD 17)

link_libraries(unioncode-GitQlient)
