# Query something from an fdd
ADD_EXECUTABLE(interaction interaction.cpp)
TARGET_LINK_LIBRARIES(interaction rti1516 fedtime1516 OpenRTI)

# No server - thread protocol, one ambassador
# ADD_TEST(rti1516/interaction-1 ${CMAKE_CURRENT_BINARY_DIR}/interaction -S0 -A10 -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
