cmake_minimum_required(VERSION 3.22)

project(1dp)

#this package should include all the source code for the file
set(SOURCES 
pongMain.cpp 
ScreenSpaceQuad.cpp 
ball.cpp 
ball.h 
OpenGLQuadricObject.h 
OpenGLQuadricObject.cpp 
1DPPhysicsObject.h 
1DPPhysicsObject.cpp
Paddle.h
Paddle.cpp
pongInputTranslator.h
pongInputTranslator.cpp
PlayerInputSample.cpp
PlayerInputSample.h
AiPaddle.h
PongParticles.h
GameTranslator.h
) 

#this is where you create "soaper_example, including any immediate link libraries"
add_executable(1DPong ${SOURCES})
#s_portable_binary_name(1DPong)
target_link_libraries(1DPong se.core se.sim se.graphics.image se.graphics.cpGL se.sim.physics)

add_executable(1DPong2P ${SOURCES})
#s_portable_binary_name(1DPong2P)
target_link_libraries(1DPong2P se.core se.sim se.graphics.image se.graphics.cpGL se.sim.physics)
set_property(TARGET 1DPong2P PROPERTY COMPILE_DEFINITIONS AI_GEN)
