# Test if dynamic linking works as required by the standard
ADD_EXECUTABLE(link-1516 link.cpp)
TARGET_LINK_LIBRARIES(link-1516 rti1516 fedtime1516)
