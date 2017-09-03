# Query something from an fdd
add_executable(fddget-13 fddget.cpp)
target_link_libraries(fddget-13 RTI-NG FedTime OpenRTI)


# FIXME also test for the privilegeToDelete?? REALLY??
# add_test(rti13/fddget-13-1 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-13" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-1.fed"
#   -o objectRoot -a privilegeToDelete -a privilegeToDelete -i interactionRoot)
add_test(rti13/fddget-13-1 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-13" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-1.fed"
  -o objectRoot -a privilegeToDelete
  -i interactionRoot)
add_test(rti13/fddget-13-2 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-13" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-1.fed"
  -o ObjectClass1 -a privilegeToDelete -a Attribute1
  -i InteractionClass1 -p Parameter1)
add_test(rti13/fddget-13-3 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-13" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-1.fed"
  -o objectRoot.ObjectClass1 -a privilegeToDelete -a objectRoot.ObjectClass1.Attribute1
  -i interactionRoot.InteractionClass1 -p interactionRoot.InteractionClass1.Parameter1)

add_test(rti13/fddget-13-4 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-13" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-1.fed"
  -o ObjectClass2 -a privilegeToDelete -a Attribute1 -a Attribute2
  -i InteractionClass2 -p Parameter1 -p Parameter2)
add_test(rti13/fddget-13-5 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-13" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-1.fed"
  -o objectRoot.ObjectClass1.ObjectClass2 -a privilegeToDelete -a objectRoot.ObjectClass1.Attribute1 -a objectRoot.ObjectClass1.ObjectClass2.Attribute2
  -i interactionRoot.InteractionClass1.InteractionClass2 -p interactionRoot.InteractionClass1.Parameter1 -p interactionRoot.InteractionClass1.InteractionClass2.Parameter2)

add_test(rti13/fddget-13-6 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-13" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-1.fed"
  -o ObjectClass3 -a privilegeToDelete -a Attribute3
  -i InteractionClass3 -p Parameter3)
add_test(rti13/fddget-13-7 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-13" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-1.fed"
  -o objectRoot.ObjectClass3 -a privilegeToDelete -a objectRoot.ObjectClass3.Attribute3
  -i interactionRoot.InteractionClass3 -p interactionRoot.InteractionClass3.Parameter3)

# Now using a different object model not containing the root object
add_test(rti13/fddget-13-11 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-13" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-2.fed"
  -o objectRoot -a privilegeToDelete
  -i interactionRoot)
add_test(rti13/fddget-13-12 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-13" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-2.fed"
  -o ObjectClass1 -a privilegeToDelete -a Attribute1
  -i InteractionClass1 -p Parameter1)
add_test(rti13/fddget-13-13 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-13" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-2.fed"
  -o ObjectClass1 -a privilegeToDelete -a ObjectClass1.Attribute1
  -i InteractionClass1 -p InteractionClass1.Parameter1)

add_test(rti13/fddget-13-14 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-13" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-2.fed"
  -o ObjectClass2 -a privilegeToDelete -a Attribute1 -a Attribute2
  -i InteractionClass2 -p Parameter1 -p Parameter2)
add_test(rti13/fddget-13-15 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-13" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-2.fed"
  -o ObjectClass1.ObjectClass2 -a privilegeToDelete -a ObjectClass1.Attribute1 -a ObjectClass1.ObjectClass2.Attribute2
  -i InteractionClass1.InteractionClass2 -p InteractionClass1.Parameter1 -p InteractionClass1.InteractionClass2.Parameter2)

add_test(rti13/fddget-13-16 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-13" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-2.fed"
  -o ObjectClass3 -a privilegeToDelete -a Attribute3
  -i InteractionClass3 -p Parameter3)
add_test(rti13/fddget-13-17 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-13" -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-2.fed"
  -o ObjectClass3 -a privilegeToDelete -a ObjectClass3.Attribute3
  -i InteractionClass3 -p InteractionClass3.Parameter3)


add_test(rti13/fddget-13-20 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fddget-13" -D -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd-2.fed"
  -o ObjectClass3 -a privilegeToDelete -a Attribute3
  -i InteractionClass3 -p Parameter3)
