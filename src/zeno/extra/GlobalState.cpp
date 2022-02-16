#include <zeno/extra/GlobalState.h>
#include <zeno/utils/logger.h>
#include <mutex>

namespace zeno {

struct GlobalState::Impl {
    struct FrameData {
        std::vector<std::shared_ptr<IObject>> view_objects;
    };

    std::vector<FrameData> frames;
    std::mutex mtx;
};

ZENO_API GlobalState state;

ZENO_API GlobalState::GlobalState() : m_impl(std::make_unique<Impl>()) {}
ZENO_API GlobalState::~GlobalState() = default;

ZENO_API bool GlobalState::substepBegin() {
    if (has_substep_executed) {
        if (!time_step_integrated)
            return false;
    }
    if (has_frame_completed)
        return false;
    return true;
}

ZENO_API void GlobalState::substepEnd() {
    substepid++;
    has_substep_executed = true;
}

ZENO_API void GlobalState::frameBegin() {
    has_frame_completed = false;
    has_substep_executed = false;
    time_step_integrated = false;
    frame_time_elapsed = 0;
    m_impl->frames.emplace_back();
}

ZENO_API void GlobalState::frameEnd() {
    frameid++;
}

ZENO_API void GlobalState::addViewObject(std::shared_ptr<IObject> const &object) {
    std::lock_guard lck(m_impl->mtx);
    if (frameid >= m_impl->frames.size()) {
        log_warn("bad frameid in addViewObject");
        return;
    }
    m_impl->frames[frameid].view_objects.push_back(object);
}

ZENO_API void GlobalState::clearState() {
    std::lock_guard lck(m_impl->mtx);
    m_impl->frames.clear();
    frameid = 0;
    frameid = 0;
    substepid = 0;
    frame_time = 0.03f;
    frame_time_elapsed = 0;
    has_frame_completed = false;
    has_substep_executed = false;
    time_step_integrated = false;
}

ZENO_API int GlobalState::countFrames() {
    std::lock_guard lck(m_impl->mtx);
    return m_impl->frames.size();
}

ZENO_API std::vector<std::shared_ptr<IObject>> GlobalState::getViewObjects(int frame_) {
    std::lock_guard lck(m_impl->mtx);
    if (frame_ < 0 || frame_ >= m_impl->frames.size()) return {};
    return m_impl->frames[frame_].view_objects;
}

}