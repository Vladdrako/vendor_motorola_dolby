/*
 * Copyright 2018 The Android Open Source Project
 * Modified for Dolby Codec2 Service (Android 16 Porting)
 */

#define LOG_TAG "vendor.dolby.media.c2-service"

#include <android-base/logging.h>
#include <util/C2InterfaceHelper.h>
#include <C2Component.h>
#include <C2Config.h>

// HIDL
#include <binder/ProcessState.h>
#include <codec2/hidl/1.0/ComponentStore.h>
#include <hidl/HidlTransportSupport.h>

namespace android {
    // Експортована функція з libcodec2_store_dolby.so
    std::shared_ptr<C2ComponentStore> GetCodec2DolbyComponentStore();
}

static constexpr int kThreadCount = 8;

void runHidlService() {
    using namespace ::android;

    ProcessState::initWithDriver("/dev/vndbinder");
    ProcessState::self()->startThreadPool();
    hardware::configureRpcThreadpool(kThreadCount, true /* callerWillJoin */);

    {
        using namespace ::android::hardware::media::c2::V1_0;
        sp<IComponentStore> store;

        LOG(INFO) << "Instantiating Dolby Codec2 HIDL IComponentStore service...";

        std::shared_ptr<C2ComponentStore> dolbyStore = android::GetCodec2DolbyComponentStore();

        if (!dolbyStore) {
            LOG(ERROR) << "Failed to load Dolby C2 Store from library!";
            return;
        }

        store = new utils::ComponentStore(dolbyStore);

        if (store == nullptr) {
            LOG(ERROR) << "Cannot create Codec2's HIDL IComponentStore service wrapper.";
        } else {
            constexpr char const* serviceName = "dolby";
            if (store->registerAsService(serviceName) != OK) {
                LOG(ERROR) << "Cannot register Dolby HIDL C2 service with name: \""
                           << serviceName << "\".";
            } else {
                LOG(INFO) << "Dolby HIDL Codec2 service registered successfully: \"" 
                          << serviceName << "\".";
            }
        }
    }

    hardware::joinRpcThreadpool();
}

int main(int /* argc */, char** /* argv */) {
    LOG(INFO) << "Dolby C2 HIDL 1.0 Service starting...";

    signal(SIGPIPE, SIG_IGN);

    // Minijail тимчасово вимкнено для тестування
    // android::SetUpMinijail(kBaseSeccompPolicyPath, kExtSeccompPolicyPath);

    runHidlService();
    return 0;
}
