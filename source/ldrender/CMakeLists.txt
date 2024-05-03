# codeshaunted - ldrender
# source/ldrender/CMakeLists.txt
# ldrender source CMake file
# Copyright 2024 codeshaunted
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http:#www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(LDRENDER_SOURCE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/main.cc"
	"${CMAKE_CURRENT_SOURCE_DIR}/utilities.cc"
	"${CMAKE_CURRENT_SOURCE_DIR}/ldraw.cc")

set(LDRENDER_INCLUDE_DIRECTORIES
	"${CMAKE_SOURCE_DIR}/include/ldrender"
	"${PROJECT_BINARY_DIR}/source/ldrender")

set(LDRENDER_LINK_LIBRARIES)

set(LDRENDER_COMPILE_DEFINITIONS)

add_executable(ldrender ${LDRENDER_SOURCE_FILES})

target_include_directories(ldrender PUBLIC ${LDRENDER_INCLUDE_DIRECTORIES})

target_link_libraries(ldrender PUBLIC ${LDRENDER_LINK_LIBRARIES})

target_compile_definitions(ldrender PUBLIC ${LDRENDER_COMPILE_DEFINITIONS})