# Query something from an fdd
ADD_EXECUTABLE(interaction-1516 interaction.cpp)
SET_PROPERTY(TARGET interaction-1516 PROPERTY OUTPUT_NAME interaction)
TARGET_LINK_LIBRARIES(interaction-1516 rti1516 fedtime1516 OpenRTI)

# No server - thread protocol, one ambassador
ADD_TEST(rti1516/interaction-1516-1 ${CMAKE_CURRENT_BINARY_DIR}/interaction -S0 -A1 -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-1516-2 ${CMAKE_CURRENT_BINARY_DIR}/interaction -S0 -A10 -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
ADD_TEST(rti1516/interaction-1516-3 ${CMAKE_CURRENT_BINARY_DIR}/interaction -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-1516-4 ${CMAKE_CURRENT_BINARY_DIR}/interaction -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-1516-5 ${CMAKE_CURRENT_BINARY_DIR}/interaction -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
