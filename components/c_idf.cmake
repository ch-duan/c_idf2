cmake_minimum_required(VERSION 3.0.0)

file(GLOB_RECURSE C_IDF_SRC "${CMAKE_CURRENT_LIST_DIR}/algorithm/*.c**"
  "${CMAKE_CURRENT_LIST_DIR}/button/*.c**"
  "${CMAKE_CURRENT_LIST_DIR}/heap/*.c**"
  "${CMAKE_CURRENT_LIST_DIR}/crypto/*.c**"
  "${CMAKE_CURRENT_LIST_DIR}/encode/*.c**"
  "${CMAKE_CURRENT_LIST_DIR}/delay/*.c**"
  "${CMAKE_CURRENT_LIST_DIR}/flash/*.c**"
  "${CMAKE_CURRENT_LIST_DIR}/vfs/*.c**"
  "${CMAKE_CURRENT_LIST_DIR}/ota/*.c**"
  "${CMAKE_CURRENT_LIST_DIR}/log/*.c**"
  "${CMAKE_CURRENT_LIST_DIR}/log/*.S"
  "${CMAKE_CURRENT_LIST_DIR}/message_queue/*.c"
  "${CMAKE_CURRENT_LIST_DIR}/mutex/*.c"
  "${CMAKE_CURRENT_LIST_DIR}/string_tools/*.c"
  "${CMAKE_CURRENT_LIST_DIR}/VIDEO/src/*.c"
  "${CMAKE_CURRENT_LIST_DIR}/w25qxx/*.c"
  "${CMAKE_CURRENT_LIST_DIR}/ws2812/*.c"
  "${CMAKE_CURRENT_LIST_DIR}/uart_idle_rx/*.c"
  "${CMAKE_CURRENT_LIST_DIR}/QMI8658/*.c"
)

set(C_IDF_SRC ${C_IDF_SRC} "${CMAKE_CURRENT_LIST_DIR}/cm_backtrace/cm_backtrace.c"
  "${CMAKE_CURRENT_LIST_DIR}/lwrb/lwrb/src/lwrb/lwrb.c"
  "${CMAKE_CURRENT_LIST_DIR}/littlefs/lfs.c"
  "${CMAKE_CURRENT_LIST_DIR}/littlefs/lfs_util.c"
)
set(C_IDF_INCLUDES
  ${CMAKE_CURRENT_LIST_DIR}/
  ${CMAKE_CURRENT_LIST_DIR}/littlefs
  ${CMAKE_CURRENT_LIST_DIR}/w25qxx/src
  ${CMAKE_CURRENT_LIST_DIR}/w25qxx/interface
  ${CMAKE_CURRENT_LIST_DIR}/cm_backtrace
  ${CMAKE_CURRENT_LIST_DIR}/w25qxx
  ${CMAKE_CURRENT_LIST_DIR}/encode
  ${CMAKE_CURRENT_LIST_DIR}/heap/lv_mem
  ${CMAKE_CURRENT_LIST_DIR}/heap
  ${CMAKE_CURRENT_LIST_DIR}/VIDEO
  ${CMAKE_CURRENT_LIST_DIR}/VIDEO/Inc
  ${CMAKE_CURRENT_LIST_DIR}/delay
  ${CMAKE_CURRENT_LIST_DIR}/ota
  ${CMAKE_CURRENT_LIST_DIR}/log
  ${CMAKE_CURRENT_LIST_DIR}/log/rtt
  ${CMAKE_CURRENT_LIST_DIR}/QMI8658
  ${CMAKE_CURRENT_LIST_DIR}/vfs
  ${CMAKE_CURRENT_LIST_DIR}/flash/nor_flash
  ${CMAKE_CURRENT_LIST_DIR}/sys
  ${CMAKE_CURRENT_LIST_DIR}/crypto
  ${CMAKE_CURRENT_LIST_DIR}/button
  ${CMAKE_CURRENT_LIST_DIR}/mutex
  ${CMAKE_CURRENT_LIST_DIR}/message_queue
  ${CMAKE_CURRENT_LIST_DIR}/algorithm/filter
  ${CMAKE_CURRENT_LIST_DIR}/uart_idle_rx
  ${CMAKE_CURRENT_LIST_DIR}/lwrb/lwrb/src/include
  ${CMAKE_CURRENT_LIST_DIR}/lwrb/lwrb/src/include/lwrb
  ${CMAKE_CURRENT_LIST_DIR}/ws2812
)

