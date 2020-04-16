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

namespace cs::core {
class Settings;
class SolarSystem;
} // namespace cs::core

namespace csp::simplebodies {

/// This is just a sphere with a texture, attached to the given SPICE frame. The texture should be
/// in equirectangular projection.
class SimpleBody : public cs::scene::CelestialBody, public IVistaOpenGLDraw {
 public:
  SimpleBody(std::shared_ptr<cs::core::Settings> settings,
      std::shared_ptr<cs::core::SolarSystem> solarSystem, std::string const& sTexture,
      std::string const& sCenterName, std::string const& sFrameName, double tStartExistence,
      double tEndExistence);

  SimpleBody(SimpleBody const& other) = delete;
  SimpleBody(SimpleBody&& other)      = delete;

  SimpleBody& operator=(SimpleBody const& other) = delete;
  SimpleBody& operator=(SimpleBody&& other) = delete;

  ~SimpleBody();

  /// The sun object is used for lighting computation.
  void setSun(std::shared_ptr<const cs::scene::CelestialObject> const& sun);

  /// Interface implementation of the IntersectableObject, which is a base class of
  /// CelestialBody.
  bool getIntersection(
      glm::dvec3 const& rayOrigin, glm::dvec3 const& rayDir, glm::dvec3& pos) const override;

  /// Interface implementation of CelestialBody.
  double     getHeight(glm::dvec2 lngLat) const override;
  glm::dvec3 getRadii() const override;

  /// Interface implementation of IVistaOpenGLDraw.
  bool Do() override;
  bool GetBoundingBox(VistaBoundingBox& bb) override;

 private:
  std::shared_ptr<cs::core::Settings>    mSettings;
  std::shared_ptr<cs::core::SolarSystem> mSolarSystem;

  std::unique_ptr<VistaTexture> mTexture;

  VistaGLSLShader        mShader;
  VistaVertexArrayObject mSphereVAO;
  VistaBufferObject      mSphereVBO;
  VistaBufferObject      mSphereIBO;

  std::shared_ptr<const cs::scene::CelestialObject> mSun;

  glm::dvec3 mRadii;

  bool mShaderDirty              = true;
  int  mEnableLightingConnection = -1;
  int  mEnableHDRConnection      = -1;

  static const char* SPHERE_VERT;
  static const char* SPHERE_FRAG;
};

} // namespace csp::simplebodies

#endif // CSP_SIMPLE_BODIES_SIMPLE_PLANET_HPP
