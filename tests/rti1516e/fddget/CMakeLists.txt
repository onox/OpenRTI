# Query something from an fdd
ADD_EXECUTABLE(fddget-1516e fddget.cpp)
TARGET_LINK_LIBRARIES(fddget-1516e rti1516e fedtime1516e OpenRTI)


# FIXME also test for the privilegeToDelete?? REALLY??
# ADD_TEST(rti1516e/fddget-1516e-1 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-1516e" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-1.xml"
#   -o HLAobjectRoot -a HLAprivilegeToDeleteObject -a privilegeToDelete -i HLAinteractionRoot)
ADD_TEST(rti1516e/fddget-1516e-1 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-1516e" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-1.xml"
  -o HLAobjectRoot -a HLAprivilegeToDeleteObject
  -i HLAinteractionRoot)
ADD_TEST(rti1516e/fddget-1516e-2 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-1516e" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-1.xml"
  -o ObjectClass1 -a HLAprivilegeToDeleteObject -a Attribute1
  -i InteractionClass1 -p Parameter1)
ADD_TEST(rti1516e/fddget-1516e-3 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-1516e" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-1.xml"
  -o HLAobjectRoot.ObjectClass1 -a HLAprivilegeToDeleteObject -a HLAobjectRoot.ObjectClass1.Attribute1
  -i HLAinteractionRoot.InteractionClass1 -p HLAinteractionRoot.InteractionClass1.Parameter1)

ADD_TEST(rti1516e/fddget-1516e-4 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-1516e" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-1.xml"
  -o ObjectClass2 -a HLAprivilegeToDeleteObject -a Attribute1 -a Attribute2
  -i InteractionClass2 -p Parameter1 -p Parameter2)
ADD_TEST(rti1516e/fddget-1516e-5 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-1516e" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-1.xml"
  -o HLAobjectRoot.ObjectClass1.ObjectClass2 -a HLAprivilegeToDeleteObject -a HLAobjectRoot.ObjectClass1.Attribute1 -a HLAobjectRoot.ObjectClass1.ObjectClass2.Attribute2
  -i HLAinteractionRoot.InteractionClass1.InteractionClass2 -p HLAinteractionRoot.InteractionClass1.Parameter1 -p HLAinteractionRoot.InteractionClass1.InteractionClass2.Parameter2)

ADD_TEST(rti1516e/fddget-1516e-6 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-1516e" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-1.xml"
  -o ObjectClass3 -a HLAprivilegeToDeleteObject -a Attribute3
  -i InteractionClass3 -p Parameter3)
ADD_TEST(rti1516e/fddget-1516e-7 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-1516e" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-1.xml"
  -o HLAobjectRoot.ObjectClass3 -a HLAprivilegeToDeleteObject -a HLAobjectRoot.ObjectClass3.Attribute3
  -i HLAinteractionRoot.InteractionClass3 -p HLAinteractionRoot.InteractionClass3.Parameter3)

# Now using a different object model not containing the root object
ADD_TEST(rti1516e/fddget-1516e-11 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-1516e" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-2.xml"
  -o HLAobjectRoot -a HLAprivilegeToDeleteObject
  -i HLAinteractionRoot)
ADD_TEST(rti1516e/fddget-1516e-12 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-1516e" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-2.xml"
  -o ObjectClass1 -a HLAprivilegeToDeleteObject -a Attribute1
  -i InteractionClass1 -p Parameter1)
ADD_TEST(rti1516e/fddget-1516e-13 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-1516e" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-2.xml"
  -o ObjectClass1 -a HLAprivilegeToDeleteObject -a ObjectClass1.Attribute1
  -i InteractionClass1 -p InteractionClass1.Parameter1)

ADD_TEST(rti1516e/fddget-1516e-14 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-1516e" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-2.xml"
  -o ObjectClass2 -a HLAprivilegeToDeleteObject -a Attribute1 -a Attribute2
  -i InteractionClass2 -p Parameter1 -p Parameter2)
ADD_TEST(rti1516e/fddget-1516e-15 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-1516e" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-2.xml"
  -o ObjectClass1.ObjectClass2 -a HLAprivilegeToDeleteObject -a ObjectClass1.Attribute1 -a ObjectClass1.ObjectClass2.Attribute2
  -i InteractionClass1.InteractionClass2 -p InteractionClass1.Parameter1 -p InteractionClass1.InteractionClass2.Parameter2)

ADD_TEST(rti1516e/fddget-1516e-16 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-1516e" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-2.xml"
  -o ObjectClass3 -a HLAprivilegeToDeleteObject -a Attribute3
  -i InteractionClass3 -p Parameter3)
ADD_TEST(rti1516e/fddget-1516e-17 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-1516e" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-2.xml"
  -o ObjectClass3 -a HLAprivilegeToDeleteObject -a ObjectClass3.Attribute3
  -i InteractionClass3 -p InteractionClass3.Parameter3)
