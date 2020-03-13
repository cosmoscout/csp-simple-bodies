////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SimpleBody.hpp"

#include "../../../src/cs-core/GraphicsEngine.hpp"
#include "../../../src/cs-core/SolarSystem.hpp"
#include "../../../src/cs-graphics/TextureLoader.hpp"
#include "../../../src/cs-utils/FrameTimings.hpp"
#include "../../../src/cs-utils/utils.hpp"

#include <VistaMath/VistaBoundingBox.h>
#include <VistaOGLExt/VistaOGLUtils.h>

#include <glm/gtc/type_ptr.hpp>

namespace csp::simplebodies {

////////////////////////////////////////////////////////////////////////////////////////////////////

const unsigned GRID_RESOLUTION_X = 200;
const unsigned GRID_RESOLUTION_Y = 100;

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string SimpleBody::SPHERE_VERT = R"(
uniform vec3 uSunDirection;
uniform vec3 uRadii;
uniform mat4 uMatModelView;
uniform mat4 uMatProjection;

// inputs
layout(location = 0) in vec2 iGridPos;

// outputs
out vec2 vTexCoords;
out vec3 vPosition;
out vec3 vCenter;
out vec2 vLonLat;

const float PI = 3.141592654;

void main()
{
    vTexCoords = vec2(iGridPos.x, 1-iGridPos.y);
    vLonLat.x = iGridPos.x * 2.0 * PI;
    vLonLat.y = (iGridPos.y-0.5) * PI;
    vPosition = uRadii * vec3(
        -sin(vLonLat.x) * cos(vLonLat.y),
        -cos(vLonLat.y+PI*0.5),
        -cos(vLonLat.x) * cos(vLonLat.y)
    );
    vPosition   = (uMatModelView * vec4(vPosition, 1.0)).xyz;
    vCenter     = (uMatModelView * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    gl_Position =  uMatProjection * vec4(vPosition, 1);

    if (gl_Position.w > 0) {
      gl_Position /= gl_Position.w;
      if (gl_Position.z >= 1) {
        gl_Position.z = 0.999999;
      }
    }
}
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string SimpleBody::SPHERE_FRAG = R"(
uniform vec3 uSunDirection;
uniform sampler2D uSurfaceTexture;
uniform float uAmbientBrightness;
uniform float uSunIlluminance;
uniform float uFarClip;

// inputs
in vec2 vTexCoords;
in vec3 vSunDirection;
in vec3 vPosition;
in vec3 vCenter;
in vec2 vLonLat;

// outputs
layout(location = 0) out vec3 oColor;

vec3 SRGBtoLINEAR(vec3 srgbIn)
{
  vec3 bLess = step(vec3(0.04045),srgbIn);
  return mix( srgbIn/vec3(12.92), pow((srgbIn+vec3(0.055))/vec3(1.055),vec3(2.4)), bLess );
}
    
void main()
{
    oColor = texture(uSurfaceTexture, vTexCoords).rgb;

    #ifdef ENABLE_HDR
      oColor = SRGBtoLINEAR(oColor);
    #endif

    oColor = oColor * uSunIlluminance;

    #ifdef ENABLE_LIGHTING
      vec3 normal = normalize(vPosition - vCenter);
      float light = max(dot(normal, uSunDirection), 0.0);
      oColor = mix(oColor*uAmbientBrightness, oColor, light);
    #endif

    gl_FragDepth = length(vPosition) / uFarClip;
}
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

SimpleBody::SimpleBody(std::shared_ptr<cs::core::GraphicsEngine> const& graphicsEngine,
    std::shared_ptr<cs::core::SolarSystem> const& solarSystem, std::string const& sTexture,
    std::string const& sCenterName, std::string const& sFrameName, double tStartExistence,
    double tEndExistence)
    : cs::scene::CelestialBody(sCenterName, sFrameName, tStartExistence, tEndExistence)
    , mGraphicsEngine(graphicsEngine)
    , mSolarSystem(solarSystem)
    , mTexture(cs::graphics::TextureLoader::loadFromFile(sTexture))
    , mRadii(cs::core::SolarSystem::getRadii(sCenterName)) {
  pVisibleRadius = mRadii[0];

  // create sphere grid geometry
  std::vector<float>    vertices(GRID_RESOLUTION_X * GRID_RESOLUTION_Y * 2);
  std::vector<unsigned> indices((GRID_RESOLUTION_X - 1) * (2 + 2 * GRID_RESOLUTION_Y));

  for (int x = 0; x < GRID_RESOLUTION_X; ++x) {
    for (int y = 0; y < GRID_RESOLUTION_Y; ++y) {
      vertices[(x * GRID_RESOLUTION_Y + y) * 2 + 0] = 1.f / (GRID_RESOLUTION_X - 1) * x;
      vertices[(x * GRID_RESOLUTION_Y + y) * 2 + 1] = 1.f / (GRID_RESOLUTION_Y - 1) * y;
    }
  }

  int index = 0;

  for (int x = 0; x < GRID_RESOLUTION_X - 1; ++x) {
    indices[index++] = x * GRID_RESOLUTION_Y;
    for (int y = 0; y < GRID_RESOLUTION_Y; ++y) {
      indices[index++] = x * GRID_RESOLUTION_Y + y;
      indices[index++] = (x + 1) * GRID_RESOLUTION_Y + y;
    }
    indices[index] = indices[index - 1];
    ++index;
  }

  mSphereVAO.Bind();

  mSphereVBO.Bind(GL_ARRAY_BUFFER);
  mSphereVBO.BufferData(vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

  mSphereIBO.Bind(GL_ELEMENT_ARRAY_BUFFER);
  mSphereIBO.BufferData(indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);

  mSphereVAO.EnableAttributeArray(0);
  mSphereVAO.SpecifyAttributeArrayFloat(
      0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0, &mSphereVBO);

  mSphereVAO.Release();
  mSphereIBO.Release();
  mSphereVBO.Release();

  mEnableLightingConnection =
      mGraphicsEngine->pEnableLighting.onChange().connect([this](bool) { mShaderDirty = true; });
  mEnableHDRConnection =
      mGraphicsEngine->pEnableHDR.onChange().connect([this](bool) { mShaderDirty = true; });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SimpleBody::~SimpleBody() {
  mGraphicsEngine->pEnableLighting.onChange().disconnect(mEnableLightingConnection);
  mGraphicsEngine->pEnableHDR.onChange().disconnect(mEnableHDRConnection);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SimpleBody::setSun(std::shared_ptr<const cs::scene::CelestialObject> const& sun) {
  mSun = sun;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SimpleBody::getIntersection(
    glm::dvec3 const& rayOrigin, glm::dvec3 const& rayDir, glm::dvec3& pos) const {

  glm::dmat4 transform = glm::inverse(getWorldTransform());

  // Transform ray into planet coordinate system
  glm::dvec4 origin(rayOrigin, 1.0);
  origin = transform * origin;

  glm::dvec4 direction(rayDir, 0.0);
  direction = transform * direction;
  direction = glm::normalize(direction);

  double b    = glm::dot(origin, direction);
  double c    = glm::dot(origin, origin) - mRadii[0] * mRadii[0];
  double fDet = b * b - c;

  if (fDet < 0.0) {
    return false;
  }

  fDet = std::sqrt(fDet);
  pos  = (origin + direction * (-b - fDet));

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double SimpleBody::getHeight(glm::dvec2 lngLat) const {
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

glm::dvec3 SimpleBody::getRadii() const {
  return mRadii;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SimpleBody::Do() {
  if (!getIsInExistence() || !pVisible.get()) {
    return true;
  }

  cs::utils::FrameTimings::ScopedTimer timer("Simple Planets");

  if (mShaderDirty) {
    delete mShader;
    mShader = new VistaGLSLShader();

    // create sphere shader
    std::string defines = "#version 330\n";

    if (mGraphicsEngine->pEnableHDR.get()) {
      defines += "#define ENABLE_HDR\n";
    }

    if (mGraphicsEngine->pEnableLighting.get()) {
      defines += "#define ENABLE_LIGHTING\n";
    }

    mShader->InitVertexShaderFromString(defines + SPHERE_VERT);
    mShader->InitFragmentShaderFromString(defines + SPHERE_FRAG);
    mShader->Link();

    mShaderDirty = false;
  }

  // set uniforms
  mShader->Bind();

  glm::vec3 sunDirection(1, 0, 0);
  float     sunIlluminance(1.f);
  float     ambientBrightness(mGraphicsEngine->pAmbientBrightness.get());

  if (getCenterName() == "Sun") {
    if (mGraphicsEngine->pEnableHDR.get()) {
      double sceneScale = 1.0 / mSolarSystem->getObserver().getAnchorScale();
      sunIlluminance    = mSolarSystem->pSunLuminousPower.get() /
                       (sceneScale * sceneScale * mRadii[0] * mRadii[0] * 4.0 * glm::pi<double>());
    }

    ambientBrightness = 1.0f;

  } else if (mSun) {
    if (mGraphicsEngine->pEnableHDR.get()) {
      sunIlluminance = mSolarSystem->getSunIlluminance(getWorldTransform()[3]);
    }

    sunDirection = mSolarSystem->getSunDirection(getWorldTransform()[3]);
  }

  mShader->SetUniform(mShader->GetUniformLocation("uSunDirection"), sunDirection[0],
      sunDirection[1], sunDirection[2]);
  mShader->SetUniform(mShader->GetUniformLocation("uSunIlluminance"), sunIlluminance);
  mShader->SetUniform(mShader->GetUniformLocation("uAmbientBrightness"), ambientBrightness);

  // get modelview and projection matrices
  GLfloat glMatMV[16], glMatP[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, &glMatMV[0]);
  glGetFloatv(GL_PROJECTION_MATRIX, &glMatP[0]);
  auto matMV = glm::make_mat4x4(glMatMV) * glm::mat4(getWorldTransform());
  glUniformMatrix4fv(
      mShader->GetUniformLocation("uMatModelView"), 1, GL_FALSE, glm::value_ptr(matMV));
  glUniformMatrix4fv(mShader->GetUniformLocation("uMatProjection"), 1, GL_FALSE, glMatP);

  mShader->SetUniform(mShader->GetUniformLocation("uSurfaceTexture"), 0);
  mShader->SetUniform(
      mShader->GetUniformLocation("uRadii"), (float)mRadii[0], (float)mRadii[0], (float)mRadii[0]);
  mShader->SetUniform(
      mShader->GetUniformLocation("uFarClip"), cs::utils::getCurrentFarClipDistance());

  mTexture->Bind(GL_TEXTURE0);

  // draw
  mSphereVAO.Bind();
  glDrawElements(GL_TRIANGLE_STRIP, (GRID_RESOLUTION_X - 1) * (2 + 2 * GRID_RESOLUTION_Y),
      GL_UNSIGNED_INT, nullptr);
  mSphereVAO.Release();

  // clean up
  mTexture->Unbind(GL_TEXTURE0);

  mShader->Release();

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SimpleBody::GetBoundingBox(VistaBoundingBox& bb) {
  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::simplebodies
