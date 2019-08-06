////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Plugin.hpp"

#include "../../../src/cs-core/Settings.hpp"
#include "../../../src/cs-core/SolarSystem.hpp"
#include "../../../src/cs-utils/convert.hpp"

#include <VistaKernel/GraphicsManager/VistaSceneGraph.h>
#include <VistaKernel/GraphicsManager/VistaTransformNode.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT_FN cs::core::PluginBase* create() {
  return new csp::simpleplanets::Plugin;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT_FN void destroy(cs::core::PluginBase* pluginBase) {
  delete pluginBase;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace csp::simpleplanets {

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(const nlohmann::json& j, Plugin::Settings::Planet& o) {
  o.mTexture = j.at("texture").get<std::string>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(const nlohmann::json& j, Plugin::Settings& o) {
  o.mPlanets = j.at("planets").get<std::map<std::string, Plugin::Settings::Planet>>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::init() {
  std::cout << "Loading: CosmoScout VR Plugin Simple Planets" << std::endl;

  mPluginSettings = mAllSettings->mPlugins.at("csp-simple-planets");

  for (auto const& planetSettings : mPluginSettings.mPlanets) {
    auto anchor = mAllSettings->mAnchors.find(planetSettings.first);

    if (anchor == mAllSettings->mAnchors.end()) {
      throw std::runtime_error(
          "There is no Anchor \"" + planetSettings.first + "\" defined in the settings.");
    }

    double tStartExistence = cs::utils::convert::toSpiceTime(anchor->second.mStartExistence);
    double tEndExistence   = cs::utils::convert::toSpiceTime(anchor->second.mEndExistence);

    auto planet = std::make_shared<SimplePlanet>(planetSettings.second.mTexture,
        anchor->second.mCenter, anchor->second.mFrame, tStartExistence, tEndExistence);
    mSolarSystem->registerBody(planet);

    planet->setSun(mSolarSystem->getSun());
    mSimplePlanetNodes.push_back(mSceneGraph->NewOpenGLNode(mSceneGraph->GetRoot(), planet.get()));
    mSimplePlanets.push_back(planet);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::deInit() {
  for (auto const& simplePlanet : mSimplePlanets) {
    mSolarSystem->unregisterBody(simplePlanet);
  }

  for (auto const& simplePlanetNode : mSimplePlanetNodes) {
    mSceneGraph->GetRoot()->DisconnectChild(simplePlanetNode);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::simpleplanets
