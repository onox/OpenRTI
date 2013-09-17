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


# Test for timestaped interaction delivery
ADD_EXECUTABLE(interaction-time-1516 interaction-time.cpp)
SET_PROPERTY(TARGET interaction-time-1516 PROPERTY OUTPUT_NAME interaction-time)
TARGET_LINK_LIBRARIES(interaction-time-1516 rti1516 fedtime1516 OpenRTI)

# No server - thread protocol, one ambassador
ADD_TEST(rti1516/interaction-time-1516-1 ${CMAKE_CURRENT_BINARY_DIR}/interaction-time -L0 -M0 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-2 ${CMAKE_CURRENT_BINARY_DIR}/interaction-time -L0 -M0 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
ADD_TEST(rti1516/interaction-time-1516-3 ${CMAKE_CURRENT_BINARY_DIR}/interaction-time -L0 -M0 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-4 ${CMAKE_CURRENT_BINARY_DIR}/interaction-time -L0 -M0 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-5 ${CMAKE_CURRENT_BINARY_DIR}/interaction-time -L0 -M0 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador
ADD_TEST(rti1516/interaction-time-1516-6 ${CMAKE_CURRENT_BINARY_DIR}/interaction-time -L1 -M0 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-7 ${CMAKE_CURRENT_BINARY_DIR}/interaction-time -L1 -M0 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
ADD_TEST(rti1516/interaction-time-1516-8 ${CMAKE_CURRENT_BINARY_DIR}/interaction-time -L1 -M0 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-9 ${CMAKE_CURRENT_BINARY_DIR}/interaction-time -L1 -M0 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-10 ${CMAKE_CURRENT_BINARY_DIR}/interaction-time -L1 -M0 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador
ADD_TEST(rti1516/interaction-time-1516-11 ${CMAKE_CURRENT_BINARY_DIR}/interaction-time -L0 -M1 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-12 ${CMAKE_CURRENT_BINARY_DIR}/interaction-time -L0 -M1 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
ADD_TEST(rti1516/interaction-time-1516-13 ${CMAKE_CURRENT_BINARY_DIR}/interaction-time -L0 -M1 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-14 ${CMAKE_CURRENT_BINARY_DIR}/interaction-time -L0 -M1 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-15 ${CMAKE_CURRENT_BINARY_DIR}/interaction-time -L0 -M1 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador
ADD_TEST(rti1516/interaction-time-1516-11 ${CMAKE_CURRENT_BINARY_DIR}/interaction-time -L1 -M1 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-12 ${CMAKE_CURRENT_BINARY_DIR}/interaction-time -L1 -M1 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
ADD_TEST(rti1516/interaction-time-1516-13 ${CMAKE_CURRENT_BINARY_DIR}/interaction-time -L1 -M1 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-14 ${CMAKE_CURRENT_BINARY_DIR}/interaction-time -L1 -M1 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-15 ${CMAKE_CURRENT_BINARY_DIR}/interaction-time -L1 -M1 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
