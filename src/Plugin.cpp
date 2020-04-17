////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Plugin.hpp"

#include "../../../src/cs-core/InputManager.hpp"
#include "../../../src/cs-core/Settings.hpp"
#include "../../../src/cs-core/SolarSystem.hpp"
#include "../../../src/cs-utils/logger.hpp"
#include "../../../src/cs-utils/utils.hpp"
#include "SimpleBody.hpp"
#include "logger.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT_FN cs::core::PluginBase* create() {
  return new csp::simplebodies::Plugin;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT_FN void destroy(cs::core::PluginBase* pluginBase) {
  delete pluginBase; // NOLINT(cppcoreguidelines-owning-memory)
}

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace csp::simplebodies {

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(nlohmann::json const& j, Plugin::Settings::SimpleBody& o) {
  cs::core::Settings::deserialize(j, "texture", o.mTexture);
}

void to_json(nlohmann::json& j, Plugin::Settings::SimpleBody const& o) {
  cs::core::Settings::serialize(j, "texture", o.mTexture);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(nlohmann::json const& j, Plugin::Settings& o) {
  cs::core::Settings::deserialize(j, "bodies", o.mSimpleBodies);
}

void to_json(nlohmann::json& j, Plugin::Settings const& o) {
  cs::core::Settings::serialize(j, "bodies", o.mSimpleBodies);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::init() {

  logger().info("Loading plugin...");

  mOnLoadConnection = mAllSettings->onLoad().connect([this]() { onLoad(); });
  mOnSaveConnection = mAllSettings->onSave().connect(
      [this]() { mAllSettings->mPlugins["csp-simple-bodies"] = mPluginSettings; });

  // Load settings.
  onLoad();

  logger().info("Loading done.");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::deInit() {
  logger().info("Unloading plugin...");

  for (auto const& simpleBody : mSimpleBodies) {
    mSolarSystem->unregisterBody(simpleBody);
    mInputManager->unregisterSelectable(simpleBody);
  }

  mAllSettings->onLoad().disconnect(mOnLoadConnection);
  mAllSettings->onSave().disconnect(mOnSaveConnection);

  logger().info("Unloading done.");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::onLoad() {
  // Read settings from JSON.
  from_json(mAllSettings->mPlugins.at("csp-simple-bodies"), mPluginSettings);

  // First try to re-configure existing simpleBodies.
  for (auto&& simpleBody : mSimpleBodies) {
    auto settings = mPluginSettings.mSimpleBodies.find(simpleBody->getCenterName());
    if (settings != mPluginSettings.mSimpleBodies.end()) {
      // If there are settings for this simpleBody, reconfigure it.
      auto anchor                           = mAllSettings->mAnchors.find(settings->first);
      auto [tStartExistence, tEndExistence] = anchor->second.getExistence();
      simpleBody->setStartExistence(tStartExistence);
      simpleBody->setEndExistence(tEndExistence);
      simpleBody->setFrameName(anchor->second.mFrame);
      simpleBody->configure(settings->second);
    } else {
      // Else delete it.
      mSolarSystem->unregisterBody(simpleBody);
      mInputManager->unregisterSelectable(simpleBody);
      simpleBody.reset();
    }
  }

  // Then remove all which have been set to null.
  mSimpleBodies.erase(
      std::remove_if(mSimpleBodies.begin(), mSimpleBodies.end(), [](auto const& p) { return !p; }),
      mSimpleBodies.end());

  // Then add new simpleBodies.
  for (auto const& simpleBodySettings : mPluginSettings.mSimpleBodies) {
    auto existing = std::find_if(mSimpleBodies.begin(), mSimpleBodies.end(),
        [&](auto val) { return val->getCenterName() == simpleBodySettings.first; });
    if (existing != mSimpleBodies.end()) {
      continue;
    }

    auto anchor = mAllSettings->mAnchors.find(simpleBodySettings.first);

    if (anchor == mAllSettings->mAnchors.end()) {
      throw std::runtime_error(
          "There is no Anchor \"" + simpleBodySettings.first + "\" defined in the settings.");
    }

    auto [tStartExistence, tEndExistence] = anchor->second.getExistence();

    auto simpleBody = std::make_shared<SimpleBody>(mAllSettings, mSolarSystem,
        anchor->second.mCenter, anchor->second.mFrame, tStartExistence, tEndExistence);

    simpleBody->configure(simpleBodySettings.second);

    mSolarSystem->registerBody(simpleBody);
    mInputManager->registerSelectable(simpleBody);

    mSimpleBodies.emplace_back(simpleBody);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::simplebodies
