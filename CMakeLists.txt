#==================================================================================================================================================================
#|Copyright 2017 Oscar Lara	- scarlara@hotmail.com																												   |
#|																																								   |
#|See individual libraries separate legal notices																												   |
#|																																								   |
#|Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"),				   |
#|to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,			   |
#|and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :					   |
#|																																								   |
#|The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.								   |
#|																																								   |
#|THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,			   |
#|FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,	   |
#|WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.	   |
#==================================================================================================================================================================

cmake_minimum_required (VERSION 2.6)
project (media_fundamentals)


file(GLOB media_fundamentals_SRC    
    "*.c*"
)


file(
	GLOB_RECURSE _source_list
	LIST_DIRECTORIES false
	"${PROJECT_SOURCE_DIR}/common/src/*.c*"
	"${PROJECT_SOURCE_DIR}/common/src/*.h*"
	"${PROJECT_SOURCE_DIR}/common/src/*.glsl"
)


include_directories(${PROJECT_SOURCE_DIR}/common/includes/)


foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${PROJECT_SOURCE_DIR}/common/src" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()


add_executable(media_fundamentals ${media_fundamentals_SRC} ${_source_list})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT media_fundamentals)


target_link_libraries(media_fundamentals ${PROJECT_SOURCE_DIR}/common/lib/fmod_vc.lib)
file( COPY ${PROJECT_SOURCE_DIR}/common/lib/fmod.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})	


