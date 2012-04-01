# Query something from an fdd
ADD_EXECUTABLE(fddget-13 fddget.cpp)
SET_PROPERTY(TARGET fddget-13 PROPERTY OUTPUT_NAME fddget)
TARGET_LINK_LIBRARIES(fddget-13 RTI-NG FedTime OpenRTI)
