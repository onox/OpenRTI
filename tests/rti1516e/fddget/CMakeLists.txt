# Query something from an fdd
ADD_EXECUTABLE(fddget-1516e fddget.cpp)
SET_PROPERTY(TARGET fddget-1516e PROPERTY OUTPUT_NAME fddget)
TARGET_LINK_LIBRARIES(fddget-1516e rti1516e fedtime1516e OpenRTI)
