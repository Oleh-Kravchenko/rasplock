PROJECT(rasplock)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_DEFINITIONS(-pedantic -std=gnu99 -Wall -Werror -D_GNU_SOURCE)

ADD_SUBDIRECTORY(shared/liblog liblog)

INCLUDE_DIRECTORIES(
${JANSSON_INCLUDE_DIRS}
${LIBLOG_INCLUDE_DIR}
include
)

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(JANSSON REQUIRED jansson)

ADD_EXECUTABLE(rasplock
source/main.c

include/config.h
source/config.c

include/tools/utils.h
include/tools/trim.h
source/tools/trim.c
include/tools/file.h
source/tools/file.c
)

TARGET_LINK_LIBRARIES(rasplock log_static ${JANSSON_LIBRARIES})

INSTALL(TARGETS rasplock RUNTIME DESTINATION bin)
