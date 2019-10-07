////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Plugin.hpp"

#include "SimpleBodyRenderer.hpp"

#include "../../../src/cs-core/InputManager.hpp"
#include "../../../src/cs-core/Settings.hpp"
#include "../../../src/cs-core/SolarSystem.hpp"

#include <VistaKernel/GraphicsManager/VistaSceneGraph.h>
#include <VistaKernel/GraphicsManager/VistaTransformNode.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT_FN cs::core::PluginBase* create() {
  return new csp::simplebodies::Plugin;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT_FN void destroy(cs::core::PluginBase* pluginBase) {
  delete pluginBase;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace csp::simplebodies {

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(const nlohmann::json& j, Plugin::Settings::Body& o) {
  o.mTexture = cs::core::parseProperty<std::string>("texture", j);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(const nlohmann::json& j, Plugin::Settings& o) {
  cs::core::parseSection("csp-simple-bodies",
      [&] { o.mBodies = cs::core::parseMap<std::string, Plugin::Settings::Body>("bodies", j); });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::init() {
  std::cout << "Loading: CosmoScout VR Plugin Simple Bodies" << std::endl;

  mPluginSettings = mAllSettings->mPlugins.at("csp-simple-bodies");

  for (auto const& bodySettings : mPluginSettings.mBodies) {
    auto anchor = mAllSettings->mAnchors.find(bodySettings.first);

    if (anchor == mAllSettings->mAnchors.end()) {
      throw std::runtime_error(
          "There is no Anchor \"" + bodySettings.first + "\" defined in the settings.");
    }

    auto   existence       = cs::core::getExistenceFromSettings(*anchor);
    double tStartExistence = existence.first;
    double tEndExistence   = existence.second;

    auto body = std::make_shared<SimpleBody>(bodySettings.second.mTexture, anchor->second.mCenter,
        anchor->second.mFrame, tStartExistence, tEndExistence);
    mSolarSystem->registerBody(body);
    mInputManager->registerSelectable(body);

    mSimpleBodies.push_back(body);
  }

  mBodyRenderer = std::make_unique<SimpleBodyRenderer>();
  mBodyRenderer->setBodies(mSimpleBodies);
  mBodyRenderer->setSun(mSolarSystem->getSun());
  mRendererNode = mSceneGraph->NewOpenGLNode(mSceneGraph->GetRoot(), mBodyRenderer.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::deInit() {
  for (auto const& simpleBody : mSimpleBodies) {
    mSolarSystem->unregisterBody(simpleBody);
    mInputManager->unregisterSelectable(simpleBody);
  }

  mSceneGraph->GetRoot()->DisconnectChild(mRendererNode);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::simplebodies
