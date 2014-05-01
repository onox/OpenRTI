# Test if dynamic linking works as required by the standard
ADD_EXECUTABLE(link-1516e link.cpp)
SET_PROPERTY(TARGET link-1516e PROPERTY OUTPUT_NAME link)
TARGET_LINK_LIBRARIES(link-1516e rti1516e fedtime1516e)
