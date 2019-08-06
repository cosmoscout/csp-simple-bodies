////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CSP_SIMPLE_PLANETS_SIMPLE_PLANET_HPP
#define CSP_SIMPLE_PLANETS_SIMPLE_PLANET_HPP

#include <VistaKernel/GraphicsManager/VistaOpenGLDraw.h>
#include <VistaOGLExt/VistaBufferObject.h>
#include <VistaOGLExt/VistaGLSLShader.h>
#include <VistaOGLExt/VistaTexture.h>
#include <VistaOGLExt/VistaVertexArrayObject.h>

#include "../../../src/cs-scene/CelestialBody.hpp"

namespace csp::simpleplanets {

/// This is just a sphere with a texture...
class SimplePlanet : public cs::scene::CelestialBody, public IVistaOpenGLDraw {
 public:
  SimplePlanet(std::string const& sTexture, std::string const& sCenterName,
      std::string const& sFrameName, double tStartExistence, double tEndExistence);
  ~SimplePlanet() override = default;

  void setSun(std::shared_ptr<const cs::scene::CelestialObject> const& sun);

  bool getIntersection(
      glm::dvec3 const& rayPos, glm::dvec3 const& rayDir, glm::dvec3& pos) const override;
  double     getHeight(glm::dvec2 lngLat) const override;
  glm::dvec3 getRadii() const override;

  bool Do() override;
  bool GetBoundingBox(VistaBoundingBox& bb) override;

 private:
  std::shared_ptr<VistaTexture> mTexture;

  VistaGLSLShader        mShader;
  VistaVertexArrayObject mSphereVAO;
  VistaBufferObject      mSphereVBO;
  VistaBufferObject      mSphereIBO;

  std::shared_ptr<const cs::scene::CelestialObject> mSun;

  glm::dvec3 mRadii;

  static const std::string SPHERE_VERT;
  static const std::string SPHERE_FRAG;
};
} // namespace csp::simpleplanets
#endif // CSP_SIMPLE_PLANETS_SIMPLE_PLANET_HPP
