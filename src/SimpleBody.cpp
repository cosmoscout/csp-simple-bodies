////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SimpleBody.hpp"

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
#version 440 compatibility

uniform vec3 uSunDirection;
uniform vec3 uRadii;
uniform mat4 uMatModelView;
uniform mat4 uMatProjection;

// inputs
layout(location = 0) in vec2 iGridPos;

// outputs
out vec2 vTexCoords;
out vec3 vSunDirection;
out vec3 vPosition;
out vec3 vCenter;
out vec2 vLonLat;

const float PI = 3.141592654;

void main()
{
    vTexCoords = vec2(iGridPos.x, 1-iGridPos.y);
    vSunDirection = gl_NormalMatrix * uSunDirection;
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
}
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string SimpleBody::SPHERE_FRAG = R"(
#version 440 compatibility

uniform sampler2D uSurfaceTexture;
uniform float uAmbientBrightness;
uniform float uFarClip;

// inputs
in vec2 vTexCoords;
in vec3 vSunDirection;
in vec3 vPosition;
in vec3 vCenter;
in vec2 vLonLat;

// outputs
layout(location = 0) out vec3 oColor;

void main()
{
    vec3 normal = normalize(vPosition - vCenter);
    float light = max(dot(normal, normalize(vSunDirection)), 0.0);

    oColor = texture(uSurfaceTexture, vTexCoords).rgb;
    oColor = mix(oColor*uAmbientBrightness, oColor, light);

    gl_FragDepth = length(vPosition) / uFarClip;
}
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

SimpleBody::SimpleBody(std::string const& sTexture, std::string const& sCenterName,
    std::string const& sFrameName, double tStartExistence, double tEndExistence)
    : cs::scene::CelestialBody(sCenterName, sFrameName, tStartExistence, tEndExistence)
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

  // create sphere shader
  mShader.InitVertexShaderFromString(SPHERE_VERT);
  mShader.InitFragmentShaderFromString(SPHERE_FRAG);
  mShader.Link();
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

  // set uniforms
  mShader.Bind();

  if (getCenterName() != "Sun") {
    auto sunTransform    = mSun->getWorldTransform();
    auto planetTransform = getWorldTransform();

    auto sunDirection = sunTransform[3] - planetTransform[3];

    mShader.SetUniform(mShader.GetUniformLocation("uSunDirection"), sunDirection[0],
        sunDirection[1], sunDirection[2]);
    mShader.SetUniform(mShader.GetUniformLocation("uAmbientBrightness"), 0.2f);
  } else {
    mShader.SetUniform(mShader.GetUniformLocation("uAmbientBrightness"), 1.f);
  }

  // get modelview and projection matrices
  GLfloat glMatMV[16], glMatP[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, &glMatMV[0]);
  glGetFloatv(GL_PROJECTION_MATRIX, &glMatP[0]);
  auto matMV = glm::make_mat4x4(glMatMV) * glm::mat4(getWorldTransform());
  glUniformMatrix4fv(
      mShader.GetUniformLocation("uMatModelView"), 1, GL_FALSE, glm::value_ptr(matMV));
  glUniformMatrix4fv(mShader.GetUniformLocation("uMatProjection"), 1, GL_FALSE, glMatP);

  mShader.SetUniform(mShader.GetUniformLocation("uSurfaceTexture"), 0);
  mShader.SetUniform(
      mShader.GetUniformLocation("uRadii"), (float)mRadii[0], (float)mRadii[0], (float)mRadii[0]);
  mShader.SetUniform(
      mShader.GetUniformLocation("uFarClip"), cs::utils::getCurrentFarClipDistance());

  mTexture->Bind(GL_TEXTURE0);

  // draw
  mSphereVAO.Bind();
  glDrawElements(GL_TRIANGLE_STRIP, (GRID_RESOLUTION_X - 1) * (2 + 2 * GRID_RESOLUTION_Y),
      GL_UNSIGNED_INT, nullptr);
  mSphereVAO.Release();

  // clean up
  mTexture->Unbind(GL_TEXTURE0);

  mShader.Release();

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SimpleBody::GetBoundingBox(VistaBoundingBox& bb) {
  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::simplebodies
