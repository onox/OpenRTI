# Query something from an fdd
ADD_EXECUTABLE(interaction-1516 interaction.cpp)
SET_PROPERTY(TARGET interaction-1516 PROPERTY OUTPUT_NAME interaction)
TARGET_LINK_LIBRARIES(interaction-1516 rti1516 fedtime1516 OpenRTI)

# No server - thread protocol, one ambassador
# ADD_TEST(rti1516/interaction-1516-1 ${CMAKE_CURRENT_BINARY_DIR}/interaction -S0 -A10 -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
