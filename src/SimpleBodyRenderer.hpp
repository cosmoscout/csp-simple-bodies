////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CSP_SIMPLE_BODIES_SIMPLE_BODY_REMDERER_HPP
#define CSP_SIMPLE_BODIES_SIMPLE_BODY_REMDERER_HPP

#include <VistaKernel/GraphicsManager/VistaOpenGLDraw.h>
#include <VistaOGLExt/VistaBufferObject.h>
#include <VistaOGLExt/VistaGLSLShader.h>
#include <VistaOGLExt/VistaVertexArrayObject.h>
#include <memory>

namespace cs::scene {
class CelestialObject;
}

namespace csp::simplebodies {

class SimpleBody;

class SimpleBodyRenderer : public IVistaOpenGLDraw {
 public:
  SimpleBodyRenderer();
  ~SimpleBodyRenderer() override = default;

  void setBodies(const std::vector<std::shared_ptr<SimpleBody>>& bodies);
  void setSun(std::shared_ptr<const cs::scene::CelestialObject> const& sun);

  bool Do() override;

  bool GetBoundingBox(VistaBoundingBox& bb) override;
 private:
  VistaGLSLShader        mShader;
  VistaVertexArrayObject mSphereVAO;
  VistaBufferObject      mSphereVBO;
  VistaBufferObject      mSphereIBO;

  struct {
    uint32_t matProjection;
    uint32_t surfaceTexture;
    uint32_t farClip;
    uint32_t sunDirection;
    uint32_t ambientBrightness;
    uint32_t matModelView;
    uint32_t radii;
  } mUniforms;

  std::vector<std::shared_ptr<SimpleBody>> mBodies;
  std::shared_ptr<const cs::scene::CelestialObject> mSun;
};
} // namespace csp::simplebodies

#endif // CSP_SIMPLE_BODIES_SIMPLE_BODY_REMDERER_HPP
