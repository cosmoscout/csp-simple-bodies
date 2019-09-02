////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SimpleBodyRenderer.hpp"
#include "../../../src/cs-utils/FrameTimings.hpp"
#include "../../../src/cs-utils/utils.hpp"
#include "SimpleBody.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <utility>

namespace csp::simplebodies {
const unsigned GRID_RESOLUTION_X = 200;
const unsigned GRID_RESOLUTION_Y = 100;

SimpleBodyRenderer::SimpleBodyRenderer() {
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
  mShader.InitVertexShaderFromString(
      cs::utils::loadFileContentsToString("../share/resources/shaders/SimpleBody.vert.glsl"));
  mShader.InitFragmentShaderFromString(
      cs::utils::loadFileContentsToString("../share/resources/shaders/SimpleBody.frag.glsl"));
  mShader.Link();
}

void SimpleBodyRenderer::setSun(std::shared_ptr<const cs::scene::CelestialObject> const& sun) {
  mSun = sun;
}

bool SimpleBodyRenderer::Do() {
  cs::utils::FrameTimings::ScopedTimer timer("Simple Planets");
  std::cerr << "A?" << std::endl;

  mShader.Bind();
  mSphereVAO.Bind();

  std::cerr << "B?" << std::endl;

  GLfloat glMatMV[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, &glMatMV[0]);

  GLfloat glMatP[16];
  glGetFloatv(GL_PROJECTION_MATRIX, &glMatP[0]);

  glUniformMatrix4fv(mShader.GetUniformLocation("uMatProjection"), 1, GL_FALSE, glMatP);
  mShader.SetUniform(mShader.GetUniformLocation("uSurfaceTexture"), 0);
  mShader.SetUniform(
      mShader.GetUniformLocation("uFarClip"), cs::utils::getCurrentFarClipDistance());

  std::cerr << "C?" << std::endl;
  for (const auto& body : mBodies) {
    if (body->getCenterName() != "Sun") {
      std::cerr << "D?" << std::endl;
      auto sunTransform    = mSun->getWorldTransform();
      auto planetTransform = body->getWorldTransform();

      std::cerr << "E?" << std::endl;
      auto sunDirection = sunTransform[3] - planetTransform[3];

      std::cerr << "F?" << std::endl;
      mShader.SetUniform(mShader.GetUniformLocation("uSunDirection"), sunDirection[0],
          sunDirection[1], sunDirection[2]);
      std::cerr << "G?" << std::endl;
      mShader.SetUniform(mShader.GetUniformLocation("uAmbientBrightness"), 0.2f);
    } else {
      mShader.SetUniform(mShader.GetUniformLocation("uAmbientBrightness"), 1.f);
    }

    std::cerr << "H?" << std::endl;
    // get modelview and projection matrices
    auto matMV = glm::make_mat4x4(glMatMV) * glm::mat4(body->getWorldTransform());
    glUniformMatrix4fv(
        mShader.GetUniformLocation("uMatModelView"), 1, GL_FALSE, glm::value_ptr(matMV));

    std::cerr << "I?" << std::endl;
    mShader.SetUniform(mShader.GetUniformLocation("uRadii"), (float)body->getRadii()[0],
        (float)body->getRadii()[0], (float)body->getRadii()[0]);

    std::cerr << "J?" << std::endl;
    body->getTexture()->Bind(GL_TEXTURE0);

    auto error = glGetError();
    std::cerr << glewGetErrorString(error) << std::endl;

    std::cerr << "K?" << std::endl;
    glDrawElements(GL_TRIANGLE_STRIP, (GRID_RESOLUTION_X - 1) * (2 + 2 * GRID_RESOLUTION_Y),
        GL_UNSIGNED_INT, nullptr);

    std::cerr << "L?" << std::endl;
    body->getTexture()->Unbind(GL_TEXTURE0);

    std::cerr << "M?" << std::endl;
  }

  mSphereVAO.Release();
  mShader.Release();

  return true;
}

bool SimpleBodyRenderer::GetBoundingBox(VistaBoundingBox& bb) {
  return false;
}
void SimpleBodyRenderer::setBodies(const std::vector<std::shared_ptr<SimpleBody>>& bodies) {
  mBodies = bodies;
}
}