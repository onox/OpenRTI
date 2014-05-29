ADD_EXECUTABLE(modules-1516e modules.cpp)
TARGET_LINK_LIBRARIES(modules-1516e rti1516e fedtime1516e OpenRTI)

# # No server - thread protocol, one ambassador
# ADD_TEST(rti1516e/modules-1516e-1 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/modules-1516e" -S0 -A1 -M "${RTI1516E_MIM_FILE}" -D -O "${CMAKE_CURRENT_SOURCE_DIR}")
# # No server - thread protocol, 10 ambassadors
# ADD_TEST(rti1516e/modules-1516e-2 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/modules-1516e" -S0 -A10 -M "${RTI1516E_MIM_FILE}" -D -O "${CMAKE_CURRENT_SOURCE_DIR}")
# # 1 server - rti protocol, one ambassador
# ADD_TEST(rti1516e/modules-1516e-3 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/modules-1516e" -S1 -A1 -M "${RTI1516E_MIM_FILE}" -D -O "${CMAKE_CURRENT_SOURCE_DIR}")
# # 1 server - rti protocol, 10 ambassadors
# ADD_TEST(rti1516e/modules-1516e-4 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/modules-1516e" -S1 -A10 -M "${RTI1516E_MIM_FILE}" -D -O "${CMAKE_CURRENT_SOURCE_DIR}")
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516e/modules-1516e-5 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/modules-1516e" -S5 -A10 -M "${RTI1516E_MIM_FILE}" -D -O "${CMAKE_CURRENT_SOURCE_DIR}")

# No server - thread protocol, one ambassador
ADD_TEST(rti1516e/modules-1516e-6 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/modules-1516e" -S0 -A1 -M "${RTI1516E_MIM_FILE}" -O "${CMAKE_CURRENT_SOURCE_DIR}")
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516e/modules-1516e-7 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/modules-1516e" -S0 -A10 -M "${RTI1516E_MIM_FILE}" -O "${CMAKE_CURRENT_SOURCE_DIR}")
# 1 server - rti protocol, one ambassador
ADD_TEST(rti1516e/modules-1516e-8 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/modules-1516e" -S1 -A1 -M "${RTI1516E_MIM_FILE}" -O "${CMAKE_CURRENT_SOURCE_DIR}")
# 1 server - rti protocol, 10 ambassadors
ADD_TEST(rti1516e/modules-1516e-9 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/modules-1516e" -S1 -A10 -M "${RTI1516E_MIM_FILE}" -O "${CMAKE_CURRENT_SOURCE_DIR}")
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516e/modules-1516e-10 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/modules-1516e" -S5 -A10 -M "${RTI1516E_MIM_FILE}" -O "${CMAKE_CURRENT_SOURCE_DIR}")
