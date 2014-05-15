ADD_EXECUTABLE(logical-time-1516 logical-time.cpp)
SET_PROPERTY(TARGET logical-time-1516 PROPERTY OUTPUT_NAME logical-time)
TARGET_LINK_LIBRARIES(logical-time-1516 fedtime1516 OpenRTI)

# Test the different logical times by getting the values form the logical time factory
ADD_TEST(rti1516/logical-time-1516-1 ${CMAKE_CURRENT_BINARY_DIR}/logical-time)
ADD_TEST(rti1516/logical-time-1516-2 ${CMAKE_CURRENT_BINARY_DIR}/logical-time -F HLAfloat64Time)
ADD_TEST(rti1516/logical-time-1516-3 ${CMAKE_CURRENT_BINARY_DIR}/logical-time -F HLAinteger64Time)
ADD_TEST(rti1516/logical-time-1516-4 ${CMAKE_CURRENT_BINARY_DIR}/logical-time -F certiFedTime1516)
ADD_TEST(rti1516/logical-time-1516-5 ${CMAKE_CURRENT_BINARY_DIR}/logical-time -F CERTI)

ADD_EXECUTABLE(concurrent-time-1516 concurrent-time.cpp)
SET_PROPERTY(TARGET concurrent-time-1516 PROPERTY OUTPUT_NAME concurrent-time)
TARGET_LINK_LIBRARIES(concurrent-time-1516 rti1516 fedtime1516 OpenRTI)

# This is just to see if the opaque time implementation works as well
# FIXME currently M0
ADD_TEST(rti1516/opaque-time-1516-1 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -S0 -N0 -A2 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml force-opaque-time)

# No server - thread protocol, one ambassador
# ADD_TEST(rti1516/concurrent-time-1516-1 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N0 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/concurrent-time-1516-2 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N0 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
# ADD_TEST(rti1516/concurrent-time-1516-3 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N0 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
# ADD_TEST(rti1516/concurrent-time-1516-4 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N0 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/concurrent-time-1516-5 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N0 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador
# ADD_TEST(rti1516/concurrent-time-1516-6 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N0 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/concurrent-time-1516-7 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N0 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
# ADD_TEST(rti1516/concurrent-time-1516-8 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N0 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
# ADD_TEST(rti1516/concurrent-time-1516-9 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N0 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/concurrent-time-1516-10 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N0 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-11 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N1 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-12 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N1 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-13 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N1 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-14 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N1 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-15 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N1 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-16 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N1 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-17 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N1 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-18 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N1 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-19 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N1 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-20 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N1 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador
# ADD_TEST(rti1516/concurrent-time-1516-21 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N2 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/concurrent-time-1516-22 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N2 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
# ADD_TEST(rti1516/concurrent-time-1516-23 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N2 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
# ADD_TEST(rti1516/concurrent-time-1516-24 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N2 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/concurrent-time-1516-25 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N2 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador
# ADD_TEST(rti1516/concurrent-time-1516-26 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N2 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/concurrent-time-1516-27 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N2 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
# ADD_TEST(rti1516/concurrent-time-1516-28 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N2 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
# ADD_TEST(rti1516/concurrent-time-1516-29 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N2 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/concurrent-time-1516-30 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N2 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-31 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N3 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-32 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N3 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-33 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N3 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-34 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N3 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-35 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N3 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-36 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N3 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-37 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N3 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-38 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N3 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-39 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N3 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-40 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N3 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-41 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N4 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-42 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N4 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-43 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N4 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-44 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N4 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-45 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N4 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-46 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N4 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-47 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N4 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-48 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N4 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-49 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N4 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-50 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N4 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)


# No server - thread protocol, one ambassador, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-51 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N5 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-52 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N5 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-53 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N5 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-54 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N5 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-55 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L0 -N5 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-56 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N5 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-57 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N5 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-58 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N5 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors, all within a single federation created once
# ADD_TEST(rti1516/concurrent-time-1516-59 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N5 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-60 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -I0 -L1 -N5 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)



# No server - thread protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-1 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N0 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-2 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N0 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-3 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N0 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
# ADD_TEST(rti1516/interaction-time-1516-4 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N0 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-5 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N0 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-6 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N0 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-7 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N0 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-8 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N0 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
# ADD_TEST(rti1516/interaction-time-1516-9 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N0 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-10 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N0 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-11 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N1 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-12 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N1 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-13 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N1 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
# ADD_TEST(rti1516/interaction-time-1516-14 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N1 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-15 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N1 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-16 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N1 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-17 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N1 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-18 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N1 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
# ADD_TEST(rti1516/interaction-time-1516-19 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N1 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-20 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N1 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-21 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N2 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-22 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N2 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-23 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N2 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
# ADD_TEST(rti1516/interaction-time-1516-24 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N2 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-25 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N2 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-26 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N2 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-27 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N2 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-28 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N2 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
# ADD_TEST(rti1516/interaction-time-1516-29 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N2 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-30 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N2 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-31 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N3 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-32 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N3 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-33 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N3 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
# ADD_TEST(rti1516/interaction-time-1516-34 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N3 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-35 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N3 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-36 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N3 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-37 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N3 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-38 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N3 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
# ADD_TEST(rti1516/interaction-time-1516-39 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N3 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-40 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N3 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-41 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N4 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-42 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N4 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-43 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N4 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
# ADD_TEST(rti1516/interaction-time-1516-44 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N4 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-45 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N4 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-46 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N4 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-47 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N4 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-48 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N4 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
# ADD_TEST(rti1516/interaction-time-1516-49 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N4 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-50 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N4 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)


# No server - thread protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-51 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N5 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-52 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N5 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-53 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N5 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
# ADD_TEST(rti1516/interaction-time-1516-54 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N5 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-55 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -N5 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-56 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N5 -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-57 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N5 -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
# ADD_TEST(rti1516/interaction-time-1516-58 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N5 -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
# ADD_TEST(rti1516/interaction-time-1516-59 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N5 -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/interaction-time-1516-60 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -N5 -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
