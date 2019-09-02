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
  o.mTexture = j.at("texture").get<std::string>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(const nlohmann::json& j, Plugin::Settings& o) {
  o.mBodies = j.at("bodies").get<std::map<std::string, Plugin::Settings::Body>>();
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

    double tStartExistence = cs::utils::convert::toSpiceTime(anchor->second.mStartExistence);
    double tEndExistence   = cs::utils::convert::toSpiceTime(anchor->second.mEndExistence);

    auto body = std::make_shared<SimpleBody>(bodySettings.second.mTexture, anchor->second.mCenter,
        anchor->second.mFrame, tStartExistence, tEndExistence);
    mSolarSystem->registerBody(body);
    mInputManager->registerSelectable(body);

    mSimpleBodies.push_back(body);
  }

  mBodyRenderer = SimpleBodyRenderer();
  mBodyRenderer.setBodies(mSimpleBodies);
  mBodyRenderer.setSun(mSolarSystem->getSun());
  mRendererNode = mSceneGraph->NewOpenGLNode(mSceneGraph->GetRoot(), &mBodyRenderer);
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
