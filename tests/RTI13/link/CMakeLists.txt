# Test if dynamic linking works as required by the standard
ADD_EXECUTABLE(link-13 link.cpp)
SET_PROPERTY(TARGET link-13 PROPERTY OUTPUT_NAME link)
TARGET_LINK_LIBRARIES(link-13 RTI-NG FedTime)
