# -- Project Setup ------------------------------------------------------------

project(neonpoolBTC)

# -- neonpoolBTC -------------------------------------------------------------------

set(neonpoolBTC_sources
  bf/bitvector.cpp
  bf/counter_vector.cpp
  bf/hash.cpp
  bf/bloom_filter/a2.cpp
  bf/bloom_filter/basic.cpp
  bf/bloom_filter/bitwise.cpp
  bf/bloom_filter/counting.cpp
  bf/bloom_filter/stable.cpp
  rapidjson/document.h
  rapidjson/writer.h
  rapidjson/stringbuffer.h
  rapidjson/allocators.h
  rapidjson/filereadstream.h
  helpers/primitives.h
  cpp-datetimelite/datetimelite.h
  neonpoolBTC.cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(neonpoolBTC ${neonpoolBTC_sources})
