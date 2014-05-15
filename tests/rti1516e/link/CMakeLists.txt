# Test if dynamic linking works as required by the standard
ADD_EXECUTABLE(link-1516e link.cpp)
TARGET_LINK_LIBRARIES(link-1516e rti1516e fedtime1516e)
