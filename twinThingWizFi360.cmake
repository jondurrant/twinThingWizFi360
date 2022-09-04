# Add library cpp files


add_library(twinThingWizFi360 INTERFACE)
target_sources(twinThingWizFi360 INTERFACE
#    ${CMAKE_CURRENT_LIST_DIR}/src/MQTTAgent.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/src/MQTTInterface.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/src/MQTTRouter.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/src/MQTTRouterPing.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/src/MQTTPingTask.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/src/MQTTTopicHelper.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/src/State.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/src/StateObserver.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/src/StateTemp.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/src/TwinTask.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/src/MQTTRouterTwin.cpp
#	${CMAKE_CURRENT_LIST_DIR}/src/MQTTAgentObserver.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/WifiHelper.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/IoTSockTrans.cpp
)

# Add include directory
target_include_directories(twinThingWizFi360 INTERFACE 
	${CMAKE_CURRENT_LIST_DIR}/src
	${TWIN_THING_PICO_CONFIG_PATH}
)

# Add the standard library to the build
target_link_libraries(twinThingWizFi360 INTERFACE 
	pico_stdlib 
	hardware_adc 
	json_maker 
	tiny_json
	CMSIS_FREERTOS_FILES
     WIZFI360_DRIVER_FILES
     WIZFI360_FILES
     FREERTOS_FILES
     IOT_SOCKET_FILES
     hardware_rtc
     coreMQTT
     coreMQTTAgent
	)

