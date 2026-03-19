//
//  ofxLaserDacManagerLibera.cpp
//  ofxLaser
//

#include "ofxLaserDacManagerLibera.h"

#include <algorithm>
#include <thread>
#include <utility>

using namespace ofxLaser;

namespace {

void ensureLiberaRegistrarsLinked() {
    // General strategy:
    // reference each static registrar explicitly so the linker cannot discard
    // manager registration side-effects when dead-stripping unused symbols.
    (void)&libera::avb::AvbManager::registrar;
    (void)&libera::etherdream::EtherDreamManager::registrar;
    (void)&libera::helios::HeliosManager::registrar;
    (void)&libera::idn::IdnManager::registrar;
    (void)&libera::lasercubenet::LaserCubeNetManager::registrar;
    (void)&libera::lasercubeusb::LaserCubeUsbManager::registrar;
}

std::vector<libera::avb::AvbDeviceConfiguration>
parseAvbDeviceConfigurations(const ofJson& json) {
    std::vector<libera::avb::AvbDeviceConfiguration> configs;
    if (!json.is_array()) {
        return configs;
    }

    for (const auto& entry : json) {
        if (!entry.is_object()) {
            continue;
        }

        libera::avb::AvbDeviceConfiguration config;
        if (entry.contains("deviceUid") && entry["deviceUid"].is_string()) {
            config.deviceUid = entry["deviceUid"].get<std::string>();
        }
        if (entry.contains("preferredPointRate") && entry["preferredPointRate"].is_number_unsigned()) {
            config.preferredPointRate = entry["preferredPointRate"].get<std::uint32_t>();
        }

        if (!config.deviceUid.empty()) {
            configs.push_back(std::move(config));
        }
    }

    return configs;
}

std::vector<string> parseHalfXYOutputControllers(const ofJson& json) {
    std::vector<string> controllerIds;
    if (!json.is_array()) {
        return controllerIds;
    }

    for (const auto& entry : json) {
        if (entry.is_string()) {
            controllerIds.push_back(entry.get<string>());
            continue;
        }

        // Compatibility strategy:
        // older AVB settings stored one object per controller with an explicit
        // halfXYOutput flag. Keep reading that format so existing projects do
        // not lose the workaround when migrated into DacManagerLibera.
        if (!entry.is_object()) {
            continue;
        }

        if (!entry.contains("controllerId") || !entry["controllerId"].is_string()) {
            continue;
        }
        if (!entry.contains("halfXYOutput") || !entry["halfXYOutput"].is_boolean()) {
            continue;
        }
        if (!entry["halfXYOutput"].get<bool>()) {
            continue;
        }

        controllerIds.push_back(entry["controllerId"].get<string>());
    }

    return controllerIds;
}

std::string makeFriendlySourceLabel(const std::string& sourceType,
                                    const std::string& sourceLabel) {
    // IDN devices often advertise a default "Main" service. Keep the full
    // SDK/service label for stable matching, but hide that suffix in the
    // fallback UI label so Helios PRO names stay concise.
    if (sourceType == "IDN") {
        static constexpr std::string_view mainServiceSuffix = " - Main";
        if (sourceLabel.size() > mainServiceSuffix.size() &&
            sourceLabel.compare(sourceLabel.size() - mainServiceSuffix.size(),
                                mainServiceSuffix.size(),
                                mainServiceSuffix) == 0) {
            return sourceLabel.substr(0, sourceLabel.size() - mainServiceSuffix.size());
        }
    }

    return sourceLabel;
}

bool usageStateImpliesUnavailable(libera::core::ControllerUsageState usageState) {
    switch (usageState) {
        case libera::core::ControllerUsageState::Active:
        case libera::core::ControllerUsageState::BusyExclusive:
            return true;
        case libera::core::ControllerUsageState::Unknown:
        case libera::core::ControllerUsageState::Idle:
            return false;
    }

    return false;
}

} // namespace

DacManagerLibera::DacManagerLibera() {
    // Register all builtin protocol managers before constructing the libera
    // system that snapshots the manager factory list.
    ensureLiberaRegistrarsLinked();
    liberaSystem = std::make_unique<libera::System>();
    verbose = false;

    // Discovery can block (IDN scans may take ~600ms), so run it off the UI
    // thread and keep a cached snapshot for fast reads.
    discoveryRunning.store(true);
    discoveryThread = std::thread([this] {
        discoveryLoop();
    });
}

DacManagerLibera::~DacManagerLibera() {
    exit();
}

string DacManagerLibera::makeStableId(const libera::core::ControllerInfo& info) const {
    // Stable ID format keeps one manager type ("Libera") while preserving
    // source-type uniqueness in case different backends expose the same raw ID.
    return info.type() + "::" + info.idValue();
}

std::vector<DacManagerLibera::DiscoveredInfo> DacManagerLibera::discoverInfosBlocking() {
    std::vector<DiscoveredInfo> result;
    {
        std::scoped_lock<std::mutex> lock(liberaSystemMutex);
        if (!liberaSystem) {
            return result;
        }

        std::vector<std::unique_ptr<libera::core::ControllerInfo>> discovered =
            liberaSystem->discoverControllers();
        result.reserve(discovered.size());

        for (std::unique_ptr<libera::core::ControllerInfo>& info : discovered) {
            if (!info) {
                continue;
            }

            DiscoveredInfo out;
            out.stableId = makeStableId(*info);
            out.sourceType = info->type();
            out.sourceId = info->idValue();
            out.sourceLabel = info->labelValue();
            out.maxPointRate = info->maxPointRate();
            out.usageState = info->usageState();
            out.controllerInfo = std::shared_ptr<libera::core::ControllerInfo>(std::move(info));
            result.push_back(std::move(out));
        }
    }

    return result;
}

std::vector<DacManagerLibera::DiscoveredInfo> DacManagerLibera::getCachedInfos() {
    std::scoped_lock<std::mutex> lock(discoveryCacheMutex);
    return cachedInfos;
}

void DacManagerLibera::refreshDiscoveryCache() {
    std::vector<DiscoveredInfo> freshInfos = discoverInfosBlocking();

    bool changed = false;
    {
        std::scoped_lock<std::mutex> lock(discoveryCacheMutex);
        const auto makeDiscoverySignature = [](const std::vector<DiscoveredInfo>& infos) {
            std::vector<std::pair<string, libera::core::ControllerUsageState>> signature;
            signature.reserve(infos.size());
            for (const DiscoveredInfo& info : infos) {
                signature.emplace_back(info.stableId, info.usageState);
            }
            std::sort(signature.begin(), signature.end(), [](const auto& a, const auto& b) {
                return a.first < b.first;
            });
            return signature;
        };

        // Strategy:
        // keep the old "device list changed" trigger, but also refresh when a
        // discovery-time usage flag changes so "(in use)" stays current.
        changed = makeDiscoverySignature(cachedInfos) != makeDiscoverySignature(freshInfos);
        cachedInfos = std::move(freshInfos);
    }

    if (changed) {
        dacsChanged = true;
    }
}

void DacManagerLibera::discoveryLoop() {
    const std::chrono::milliseconds sleepStep{50};
    while (discoveryRunning.load()) {
        refreshDiscoveryCache();

        std::chrono::milliseconds slept{0};
        while (discoveryRunning.load() && slept < discoveryInterval) {
            const std::chrono::milliseconds remaining = discoveryInterval - slept;
            const std::chrono::milliseconds nap = std::min(sleepStep, remaining);
            std::this_thread::sleep_for(nap);
            slept += nap;
        }
    }
}

std::shared_ptr<libera::core::LaserController>
DacManagerLibera::findOrConnectController(const DiscoveredInfo& info) {
    {
        std::scoped_lock<std::mutex> lock(controllerMutex);
        const auto existing = controllerByStableId.find(info.stableId);
        if (existing != controllerByStableId.end()) {
            return existing->second;
        }
    }

    std::shared_ptr<libera::core::LaserController> controller;
    {
        std::scoped_lock<std::mutex> lock(liberaSystemMutex);
        if (!liberaSystem || !info.controllerInfo) {
            return nullptr;
        }

        // Connection strategy:
        // discovery already ran on the background thread and produced the
        // backend-specific ControllerInfo instance for this stable id.
        // Reuse that cached object here so startup reassignment does not
        // trigger another blocking discoverControllers() on the UI thread.
        controller = liberaSystem->connectController(*info.controllerInfo);
    }

    if (!controller) {
        return nullptr;
    }

    std::scoped_lock<std::mutex> lock(controllerMutex);
    controllerByStableId[info.stableId] = controller;
    return controller;
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
    std::vector<DiscoveredInfo> infos = getCachedInfos();
    list.reserve(infos.size());

    for (const DiscoveredInfo& info : infos) {
        // Expose one ofxLaser DAC type ("Libera"), with composite stable IDs.
        list.emplace_back(getType(),
                          info.stableId,
                          "",
                          usageStateImpliesUnavailable(info.usageState));
        // Keep the stable ID for assignment/reconnect, but surface the human
        // readable libera label as the default UI name when no explicit alias
        // has been set by the user.
        if (!info.sourceLabel.empty()) {
            list.back().defaultDisplayLabel =
                makeFriendlySourceLabel(info.sourceType, info.sourceLabel);
        }
    }

    return list;
}

std::shared_ptr<DacBase> DacManagerLibera::getAndConnectToDac(const string& id) {
    // Reuse existing wrapper if already created.
    std::shared_ptr<DacBase> existingWrapper = getDacById(id);
    if (existingWrapper) {
        return existingWrapper;
    }

    std::vector<DiscoveredInfo> infos = getCachedInfos();
    auto it = std::find_if(infos.begin(), infos.end(), [&id](const DiscoveredInfo& info) {
        return info.stableId == id;
    });
    if (it == infos.end()) {
        // Keep connection non-blocking on the caller thread. If discovery has
        // not produced this device yet, the assigner will retry after the next
        // background cache refresh.
        return nullptr;
    }

    std::shared_ptr<libera::core::LaserController> controller = findOrConnectController(*it);
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

void DacManagerLibera::serialize(ofJson& json) const {
    ofJson avbJson;
    avbJson["devices"] = ofJson::array();
    for (const auto& config : libera::avb::AvbManager::configuredDevices()) {
        ofJson entry;
        entry["deviceUid"] = config.deviceUid;
        entry["preferredPointRate"] = config.preferredPointRate;
        avbJson["devices"].push_back(std::move(entry));
    }

    avbJson["halfXYOutputControllers"] = ofJson::array();
    for (const auto& controllerId : libera::avb::AvbManager::halfXYOutputControllers()) {
        avbJson["halfXYOutputControllers"].push_back(controllerId);
    }

    json["avb"] = std::move(avbJson);
}

bool DacManagerLibera::deserialize(ofJson& json) {
    std::vector<libera::avb::AvbDeviceConfiguration> avbConfigs;
    std::vector<string> halfXYOutputControllers;

    if (json.is_object() && json.contains("avb") && json["avb"].is_object()) {
        const ofJson& avbJson = json["avb"];
        avbConfigs = parseAvbDeviceConfigurations(avbJson["devices"]);
        halfXYOutputControllers = parseHalfXYOutputControllers(avbJson["halfXYOutputControllers"]);
    }

    // AVB manager state is app-level DAC config, so deserializing a new
    // project must replace the configured interfaces and per-bank Half X/Y
    // Output flags rather than merging with whatever was already active.
    libera::avb::AvbManager::setConfiguredDevices(avbConfigs);
    libera::avb::AvbManager::setHalfXYOutputControllers(halfXYOutputControllers);

    // Refresh the discovery snapshot immediately so startup reassignment sees
    // the saved AVB bank IDs before the background discovery loop wakes up.
    refreshDiscoveryCache();
    return true;
}

void DacManagerLibera::exit() {
    // DacAssigner explicitly calls manager->exit() before destruction, and the
    // manager destructor also calls exit(). Guard to keep shutdown strictly
    // one-shot and avoid duplicate teardown races.
    if (exitStarted.exchange(true)) {
        return;
    }

    discoveryRunning.store(false);
    if (discoveryThread.joinable()) {
        discoveryThread.join();
    }

    {
        std::scoped_lock<std::mutex> lock(discoveryCacheMutex);
        cachedInfos.clear();
    }

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

    {
        std::scoped_lock<std::mutex> lock(liberaSystemMutex);
        if (liberaSystem) {
            // libera::System destructor already calls shutdown().
            liberaSystem.reset();
        }
    }
}
