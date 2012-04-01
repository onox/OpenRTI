# Query something from an fdd
ADD_EXECUTABLE(fddget-1516 fddget.cpp)
SET_PROPERTY(TARGET fddget-1516 PROPERTY OUTPUT_NAME fddget)
TARGET_LINK_LIBRARIES(fddget-1516 rti1516 fedtime1516 OpenRTI)
