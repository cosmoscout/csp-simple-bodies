////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CSP_SIMPLE_BODIES_SIMPLE_PLANET_HPP
#define CSP_SIMPLE_BODIES_SIMPLE_PLANET_HPP

#include <VistaKernel/GraphicsManager/VistaOpenGLDraw.h>
#include <VistaOGLExt/VistaBufferObject.h>
#include <VistaOGLExt/VistaGLSLShader.h>
#include <VistaOGLExt/VistaTexture.h>
#include <VistaOGLExt/VistaVertexArrayObject.h>

#include "../../../src/cs-scene/CelestialBody.hpp"

namespace csp::simplebodies {

/// This is just a sphere with a texture...
class SimpleBody : public cs::scene::CelestialBody {
 public:
  SimpleBody(std::string const& sTexture, std::string const& sCenterName,
      std::string const& sFrameName, double tStartExistence, double tEndExistence);
  ~SimpleBody() override = default;

  bool getIntersection(
      glm::dvec3 const& rayOrigin, glm::dvec3 const& rayDir, glm::dvec3& pos) const override;
  double     getHeight(glm::dvec2 lngLat) const override;
  glm::dvec3 getRadii() const override;

  std::shared_ptr<VistaTexture> getTexture() const;
 private:
  std::shared_ptr<VistaTexture> mTexture;
  glm::dvec3 mRadii;
};

} // namespace csp::simplebodies

#endif // CSP_SIMPLE_BODIES_SIMPLE_PLANET_HPP
