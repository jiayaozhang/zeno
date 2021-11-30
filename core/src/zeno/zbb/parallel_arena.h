#pragma once


#include <zeno/zbb/blocked_range.h>
#include <functional>
#include <thread>
#include <vector>
#include <barrier>
#include <cstring>


ZENO_NAMESPACE_BEGIN
namespace zbb {


struct arena {
    struct task {
        std::function<void(std::size_t)> _func;

        task() = default;

        inline task(auto &&func)
            : _func(std::forward<decltype(func)>(func))
        {}

        inline void operator()(std::size_t procid) const {
            _func(procid);
        }
    };

    struct proc {
        std::jthread _thr;

        inline explicit proc(auto &&func)
            : _thr(std::forward<decltype(func)>(func))
        {}

        inline void join() {
            _thr.join();
        }
    };

    std::size_t _nprocs;
    std::vector<task> _tasks;
    std::vector<proc> _procs;

    struct _no_completion {
        constexpr void operator()() const {}
    };
    std::barrier<_no_completion> _barrier;

    explicit arena(std::size_t nprocs)
        : _nprocs(nprocs), _barrier(nprocs)
    {}

    inline void submit(task func) {
        _tasks.push_back(std::move(func));
    }

    void start() {
        for (std::size_t procid = 0; procid < _nprocs; procid++) {
            proc thr{[procid, this] {
                for (std::size_t taskid = procid; taskid < _tasks.size(); taskid += _nprocs) {
                    auto const &func = _tasks[taskid];
                    func(procid);
                }
            }};
            _procs.push_back(std::move(thr));
        }
    }

    void wait() {
        for (auto &thr: _procs) {
            thr.join();
        }
    }
};


template <class T>
static void parallel_arena(blocked_range<T> const &r, auto const &kern) {
    std::size_t nprocs = r.num_procs();
    std::size_t ngrain = r.grain_size();

    T itb = r.begin(), ite = r.end();
    arena a(nprocs);
    for (T it = r.begin(); it != r.end();) {
        T b = it;
        T e = it + ngrain;
        [[unlikely]] if (e >= r.end()) {
            e = r.end();
        }
        a.submit([=] (std::size_t procid) {
            kern([&] (auto const &body) {
                blocked_range<T> const r{b, e, procid, ngrain, nprocs};
                body(r);
            }, procid);
        });
        it = e;
    }
    a.start();
    a.wait();
}


}
ZENO_NAMESPACE_END