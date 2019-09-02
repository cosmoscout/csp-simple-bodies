# ------------------------------------------------------------------------------------------------ #
#                                This file is part of CosmoScout VR                                #
#       and may be used under the terms of the MIT license. See the LICENSE file for details.      #
#                         Copyright: (c) 2019 German Aerospace Center (DLR)                        #
# ------------------------------------------------------------------------------------------------ #

# build plugin -------------------------------------------------------------------------------------

file(GLOB SOURCE_FILES src/*.cpp)

add_library(csp-simple-bodies SHARED
  ${SOURCE_FILES}
)

target_link_libraries(csp-simple-bodies
  PUBLIC
    cs-core
)

# install plugin -----------------------------------------------------------------------------------

install(TARGETS   csp-simple-bodies DESTINATION "share/plugins")
install(DIRECTORY "textures"        DESTINATION "share/resources")
install(DIRECTORY "shaders"         DESTINATION "share/resources")
