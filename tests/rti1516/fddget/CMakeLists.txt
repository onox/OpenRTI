# Query something from an fdd
ADD_EXECUTABLE(fddget fddget.cpp)
TARGET_LINK_LIBRARIES(fddget rti1516 fedtime1516 OpenRTI)
