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
    (void)&libera::etherdream::EtherDreamManager::registrar;
    (void)&libera::helios::HeliosManager::registrar;
    (void)&libera::idn::IdnManager::registrar;
    (void)&libera::lasercubenet::LaserCubeNetManager::registrar;
    (void)&libera::lasercubeusb::LaserCubeUsbManager::registrar;
}

template <typename TInfoList>
std::vector<string> extractSortedStableIds(const TInfoList& infos) {
    std::vector<string> ids;
    ids.reserve(infos.size());
    for (const auto& info : infos) {
        ids.push_back(info.stableId);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

} // namespace

DacManagerLibera::DacManagerLibera() {
    // Register all builtin protocol managers before constructing the global
    // discovery object that snapshots the manager factory list.
    ensureLiberaRegistrarsLinked();
    liberaManager = std::make_unique<libera::core::GlobalDacManager>();
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

string DacManagerLibera::makeStableId(const libera::core::DacInfo& info) const {
    // Stable ID format keeps one manager type ("Libera") while preserving
    // source-type uniqueness in case different backends expose the same raw ID.
    return info.type() + "::" + info.idValue();
}

std::vector<DacManagerLibera::DiscoveredInfo> DacManagerLibera::discoverInfosBlocking() {
    std::vector<DiscoveredInfo> result;
    {
        std::scoped_lock<std::mutex> lock(liberaManagerMutex);
        if (!liberaManager) {
            return result;
        }

        std::vector<std::unique_ptr<libera::core::DacInfo>> discovered = liberaManager->discoverAll();
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
        // Strategy:
        // only raise dacsChanged when the device identity set changes.
        // Metadata changes are still updated in cache but do not trigger list
        // churn in the assigner UI.
        const std::vector<string> oldIds = extractSortedStableIds(cachedInfos);
        const std::vector<string> newIds = extractSortedStableIds(freshInfos);
        changed = (oldIds != newIds);
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
DacManagerLibera::findOrConnectController(const string& stableId) {
    {
        std::scoped_lock<std::mutex> lock(controllerMutex);
        const auto existing = controllerByStableId.find(stableId);
        if (existing != controllerByStableId.end()) {
            return existing->second;
        }
    }

    std::shared_ptr<libera::core::LaserController> controller;
    {
        // Discover again at connection-time so we operate on fresh device metadata
        // and keep the manager independent from cached snapshot state.
        std::scoped_lock<std::mutex> lock(liberaManagerMutex);
        if (!liberaManager) {
            return nullptr;
        }

        std::vector<std::unique_ptr<libera::core::DacInfo>> discovered = liberaManager->discoverAll();
        for (const std::unique_ptr<libera::core::DacInfo>& info : discovered) {
            if (!info || makeStableId(*info) != stableId) {
                continue;
            }

            controller = liberaManager->getAndConnectToDac(*info);
            break;
        }
    }

    if (!controller) {
        return nullptr;
    }

    std::scoped_lock<std::mutex> lock(controllerMutex);
    controllerByStableId[stableId] = controller;
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

    std::vector<DiscoveredInfo> infos = getCachedInfos();
    if (infos.empty()) {
        // If cache is still cold (e.g. app just launched), do one direct scan.
        infos = discoverInfosBlocking();
        {
            std::scoped_lock<std::mutex> lock(discoveryCacheMutex);
            cachedInfos = infos;
        }
    }
    auto it = std::find_if(infos.begin(), infos.end(), [&id](const DiscoveredInfo& info) {
        return info.stableId == id;
    });
    if (it == infos.end()) {
        // Device may have appeared after the last cache refresh.
        infos = discoverInfosBlocking();
        {
            std::scoped_lock<std::mutex> lock(discoveryCacheMutex);
            cachedInfos = infos;
        }
        it = std::find_if(infos.begin(), infos.end(), [&id](const DiscoveredInfo& info) {
            return info.stableId == id;
        });
    }
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
        std::scoped_lock<std::mutex> lock(liberaManagerMutex);
        if (liberaManager) {
            // GlobalDacManager destructor already calls close().
            // Reset directly to avoid double-close of all protocol managers.
            liberaManager.reset();
        }
    }
}
