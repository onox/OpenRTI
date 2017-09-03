# Name reservation
add_executable(objectname-1516 objectname.cpp)
target_link_libraries(objectname-1516 rti1516 fedtime1516 OpenRTI)

# Object attribute updates
add_executable(objectinstance-1516 objectinstance.cpp)
target_link_libraries(objectinstance-1516 rti1516 fedtime1516 OpenRTI)

# No server - thread protocol, one ambassador
add_test(rti1516/objectname-1516-1 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/objectname-1516" -S0 -A1 -J -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml")
# No server - thread protocol, 10 ambassadors
add_test(rti1516/objectname-1516-2 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/objectname-1516" -S0 -A10 -J -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml")
# 1 server - rti protocol, one ambassador
add_test(rti1516/objectname-1516-3 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/objectname-1516" -S1 -A1 -J -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml")
# 1 server - rti protocol, 10 ambassadors
add_test(rti1516/objectname-1516-4 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/objectname-1516" -S1 -A10 -J -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml")
# 5 servers - rti protocol, 10 ambassadors
add_test(rti1516/objectname-1516-5 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/objectname-1516" -S5 -A10 -J -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml")

# No server - thread protocol, one ambassador
add_test(rti1516/objectinstance-1516-1 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/objectinstance-1516" -S0 -A1 -J -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml")
# No server - thread protocol, 10 ambassadors
add_test(rti1516/objectinstance-1516-2 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/objectinstance-1516" -S0 -A10 -J -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml")
# 1 server - rti protocol, one ambassador
add_test(rti1516/objectinstance-1516-3 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/objectinstance-1516" -S1 -A1 -J -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml")
# 1 server - rti protocol, 10 ambassadors
add_test(rti1516/objectinstance-1516-4 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/objectinstance-1516" -S1 -A10 -J -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml")
# 5 servers - rti protocol, 10 ambassadors
add_test(rti1516/objectinstance-1516-5 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/objectinstance-1516" -S5 -A10 -J -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml")
