////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Plugin.hpp"

#include "../../../src/cs-core/InputManager.hpp"
#include "../../../src/cs-core/Settings.hpp"
#include "../../../src/cs-core/SolarSystem.hpp"
#include "../../../src/cs-utils/convert.hpp"

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

    auto   existence       = cs::utils::convert::getExistenceFromSettings(*anchor);
    double tStartExistence = existence.first;
    double tEndExistence   = existence.second;

    auto body = std::make_shared<SimpleBody>(bodySettings.second.mTexture, anchor->second.mCenter,
        anchor->second.mFrame, tStartExistence, tEndExistence);
    mSolarSystem->registerBody(body);
    mInputManager->registerSelectable(body);

    body->setSun(mSolarSystem->getSun());
    mSimpleBodyNodes.push_back(mSceneGraph->NewOpenGLNode(mSceneGraph->GetRoot(), body.get()));
    mSimpleBodies.push_back(body);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::deInit() {
  for (auto const& simpleBody : mSimpleBodies) {
    mSolarSystem->unregisterBody(simpleBody);
    mInputManager->unregisterSelectable(simpleBody);
  }

  for (auto const& simpleBodyNode : mSimpleBodyNodes) {
    mSceneGraph->GetRoot()->DisconnectChild(simpleBodyNode);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::simplebodies
