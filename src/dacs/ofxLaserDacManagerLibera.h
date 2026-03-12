//
//  ofxLaserDacManagerLibera.h
//  ofxLaser
//
//  Manager-level adapter that discovers controllers through libera and hands
//  ofxLaser a single generic DAC type ("Libera").
//
//  Strategy:
//  - Discovery and connection are centralized here.
//  - Connected libera controllers are kept in a manager-owned map.
//  - DacLibera wrappers are lightweight views over those controllers.
//

#pragma once

#include "ofxLaserDacLibera.h"
#include "ofxLaserDacManagerBase.h"

#include "libera/core/GlobalDacManager.hpp"
// Register built-in managers that are already part of the current addon build.
#include "libera/etherdream/EtherDreamManager.hpp"
#include "libera/helios/HeliosManager.hpp"
#include "libera/idn/IdnManager.hpp"
#include "libera/lasercubenet/LaserCubeNetManager.hpp"
#include "libera/lasercubeusb/LaserCubeUsbManager.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

namespace ofxLaser {

class DacManagerLibera : public DacManagerBase {
public:
    DacManagerLibera();
    ~DacManagerLibera() override;

    vector<DacData> updateDacList() override;
    std::shared_ptr<DacBase> getAndConnectToDac(const string& id) override;
    bool disconnectAndDeleteDac(const string& id) override;

    string getType() override {
        return "Libera";
    }
    void exit() override;

private:
    struct DiscoveredInfo {
        string stableId;        // "Type::RawId"
        string sourceType;      // Original libera type (EtherDream/Helios/etc)
        string sourceId;        // Original source ID from libera
        string sourceLabel;     // Human-readable label from libera
        uint32_t maxPointRate = 0;
    };

    string makeStableId(const libera::core::DacInfo& info) const;
    std::vector<DiscoveredInfo> discoverInfosBlocking();
    std::vector<DiscoveredInfo> getCachedInfos();
    void refreshDiscoveryCache();
    void discoveryLoop();
    std::shared_ptr<libera::core::LaserController> findOrConnectController(const string& stableId);
    std::shared_ptr<DacLibera> createWrapper(const DiscoveredInfo& info,
                                             const std::shared_ptr<libera::core::LaserController>& controller);

    // Lazily allocated after we force-link protocol registrars.
    // Keeping this as a pointer lets constructor code control creation order.
    std::unique_ptr<libera::core::GlobalDacManager> liberaManager;
    // Serialize access to libera's manager stack; discovery and connection both
    // touch the same manager instances.
    std::mutex liberaManagerMutex;

    // Discovery cache is produced asynchronously so UI thread reads are cheap.
    std::atomic<bool> discoveryRunning{false};
    std::atomic<bool> exitStarted{false};
    std::thread discoveryThread;
    std::chrono::milliseconds discoveryInterval{1500};
    std::mutex discoveryCacheMutex;
    std::vector<DiscoveredInfo> cachedInfos;

    // Manager-owned controller storage (option 2 pattern).
    std::mutex controllerMutex;
    std::unordered_map<string, std::shared_ptr<libera::core::LaserController>> controllerByStableId;
};

} // namespace ofxLaser
