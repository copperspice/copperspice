list(APPEND MULTIMEDIA_PUBLIC_INCLUDES
   QAbstractAudioDeviceInfo
   QAbstractAudioInput
   QAbstractAudioOutput
   QAudio
   QAudioFormat
   QAudioInput
   QAudioOutput
   QAudioDeviceInfo
   QAudioEngineFactoryInterface
   QSound
   QSoundEffect
)

list(APPEND MULTIMEDIA_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qabstractaudiodeviceinfo.h
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qabstractaudioinput.h
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qabstractaudiooutput.h
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudio.h
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudiobuffer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudiodecoder.h
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudiodeviceinfo.h
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudioformat.h
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudioinput.h
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudiooutput.h
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudioprobe.h
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudioenginefactoryinterface.h
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudiosystem.h
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudiosystemplugin.h
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qsound.h
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qsoundeffect.h
)

list(APPEND MULTIMEDIA_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudiobuffer_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudiodevicefactory_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudiohelpers_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qsamplecache_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qsoundeffect_qaudio_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qsoundeffect_pulse_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qwavedecoder_p.h
)

target_sources(CsMultimedia
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudio.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudiobuffer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudiodecoder.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudiodevicefactory.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudiodeviceinfo.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudioformat.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudiohelpers.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudioinput.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudiooutput.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudioprobe.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudiosystem.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qaudiosystemplugin.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qsamplecache_p.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qsound.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qsoundeffect.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/audio/qwavedecoder_p.cpp
)

if (PulseAudio_FOUND)

   list(APPEND EXTRA_MULTIMEDIA_CXXFLAGS
      -DQT_MULTIMEDIA_PULSEAUDIO
      -DQTM_PULSEAUDIO_DEFAULTBUFFER
   )

   target_link_libraries(CsMultimedia
      PRIVATE
      ${PULSEAUDIO_LIBRARY}
   )

   target_sources(CsMultimedia
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/audio/qsoundeffect_pulse_p.cpp
   )

else()

   list(APPEND EXTRA_MULTIMEDIA_CXXFLAGS
      -DQT_MULTIMEDIA_QAUDIO
   )

   target_sources(CsMultimedia
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/audio/qsoundeffect_qaudio_p.cpp
   )

endif()

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
   target_link_libraries(CsMultimedia
      PRIVATE
      winmm
   )

endif()

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
   list(APPEND EXTRA_MULTIMEDIA_LDFLAGS
      -framework AudioUnit
      -framework CoreAudio
      -framework AudioToolbox
   )
endif()
