//
//  ofxLaserDacManagerLibera.cpp
//  ofxLaser
//

#include "ofxLaserDacManagerLibera.h"

#include <algorithm>
#include <utility>

using namespace ofxLaser;

DacManagerLibera::DacManagerLibera() {
    verbose = false;
}

DacManagerLibera::~DacManagerLibera() {
    exit();
}

string DacManagerLibera::makeStableId(const libera::core::DacInfo& info) const {
    // Stable ID format keeps one manager type ("Libera") while preserving
    // source-type uniqueness in case different backends expose the same raw ID.
    return info.type() + "::" + info.idValue();
}

std::vector<DacManagerLibera::DiscoveredInfo> DacManagerLibera::discoverInfos() {
    std::vector<DiscoveredInfo> result;
    std::vector<std::unique_ptr<libera::core::DacInfo>> discovered = liberaManager.discoverAll();
    result.reserve(discovered.size());

    for (const std::unique_ptr<libera::core::DacInfo>& info : discovered) {
        if (!info) {
            continue;
        }

        DiscoveredInfo out;
        out.stableId = makeStableId(*info);
        out.sourceType = info->type();
        out.sourceId = info->idValue();
        out.sourceLabel = info->labelValue();
        out.maxPointRate = info->maxPointRate();
        result.push_back(std::move(out));
    }

    return result;
}

std::shared_ptr<libera::core::LaserController>
DacManagerLibera::findOrConnectController(const string& stableId) {
    {
        std::scoped_lock<std::mutex> lock(controllerMutex);
        const auto existing = controllerByStableId.find(stableId);
        if (existing != controllerByStableId.end()) {
            return existing->second;
        }
    }

    // Discover again at connection-time so we operate on fresh device metadata
    // and keep the manager independent from cached snapshot state.
    std::vector<std::unique_ptr<libera::core::DacInfo>> discovered = liberaManager.discoverAll();
    for (const std::unique_ptr<libera::core::DacInfo>& info : discovered) {
        if (!info || makeStableId(*info) != stableId) {
            continue;
        }

        std::shared_ptr<libera::core::LaserController> controller =
            liberaManager.getAndConnectToDac(*info);
        if (!controller) {
            return nullptr;
        }

        std::scoped_lock<std::mutex> lock(controllerMutex);
        controllerByStableId[stableId] = controller;
        return controller;
    }

    return nullptr;
}

std::shared_ptr<DacLibera> DacManagerLibera::createWrapper(
    const DiscoveredInfo& info,
    const std::shared_ptr<libera::core::LaserController>& controller) {
    DacLibera::Descriptor descriptor;
    descriptor.type = getType();
    descriptor.rawId = info.stableId;

    // Default conversion policy tracks current ofxLaser output convention:
    // X unchanged, Y flipped into libera's normalized coordinate space.
    descriptor.pointPolicy.flipX = false;
    descriptor.pointPolicy.flipY = true;

    std::shared_ptr<DacLibera> wrapper = std::make_shared<DacLibera>(descriptor, controller);
    if (info.maxPointRate > 0) {
        wrapper->maxPointRate = static_cast<int>(info.maxPointRate);
    }
    return wrapper;
}

vector<DacData> DacManagerLibera::updateDacList() {
    vector<DacData> list;
    std::vector<DiscoveredInfo> infos = discoverInfos();
    list.reserve(infos.size());

    for (const DiscoveredInfo& info : infos) {
        // Expose one ofxLaser DAC type ("Libera"), with composite stable IDs.
        list.emplace_back(getType(), info.stableId);
    }

    return list;
}

std::shared_ptr<DacBase> DacManagerLibera::getAndConnectToDac(const string& id) {
    // Reuse existing wrapper if already created.
    std::shared_ptr<DacBase> existingWrapper = getDacById(id);
    if (existingWrapper) {
        return existingWrapper;
    }

    std::vector<DiscoveredInfo> infos = discoverInfos();
    auto it = std::find_if(infos.begin(), infos.end(), [&id](const DiscoveredInfo& info) {
        return info.stableId == id;
    });
    if (it == infos.end()) {
        return nullptr;
    }

    std::shared_ptr<libera::core::LaserController> controller = findOrConnectController(id);
    if (!controller) {
        return nullptr;
    }

    std::shared_ptr<DacLibera> wrapper = createWrapper(*it, controller);
    dacsById[id] = wrapper;
    return wrapper;
}

bool DacManagerLibera::disconnectAndDeleteDac(const string& id) {
    // Remove and close the wrapper first.
    auto wrapperIt = dacsById.find(id);
    if (wrapperIt != dacsById.end()) {
        if (wrapperIt->second) {
            wrapperIt->second->close();
        }
        dacsById.erase(wrapperIt);
    }

    // Then drop manager-owned controller state.
    std::shared_ptr<libera::core::LaserController> controller;
    {
        std::scoped_lock<std::mutex> lock(controllerMutex);
        auto controllerIt = controllerByStableId.find(id);
        if (controllerIt != controllerByStableId.end()) {
            controller = controllerIt->second;
            controllerByStableId.erase(controllerIt);
        }
    }
    if (controller) {
        controller->stop();
    }

    return true;
}

void DacManagerLibera::exit() {
    // Close wrappers so any laser references stop producing data.
    for (auto& pair : dacsById) {
        if (pair.second) {
            pair.second->close();
        }
    }
    dacsById.clear();

    // Stop manager-owned controllers.
    std::unordered_map<string, std::shared_ptr<libera::core::LaserController>> snapshot;
    {
        std::scoped_lock<std::mutex> lock(controllerMutex);
        snapshot.swap(controllerByStableId);
    }
    for (auto& pair : snapshot) {
        if (pair.second) {
            pair.second->stop();
        }
    }

    liberaManager.close();
}
