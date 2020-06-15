/**
 *
 * @file      vpp/core/pipeline.hpp
 *
 * @brief     This is the VPP core pipeline description file
 *
 * @details   This file describes the general structure of a pipeline, i.e. the
 *            entity responsible of managing a vector of core stages. 
 *
 *            This file is part of the VPP framework (see link).
 *
 * @author    Olivier Stoltz-Douchet <ezdayo@gmail.com>
 *
 * @copyright (c) 2019-2020 Olivier Stoltz-Douchet
 * @license   http://opensource.org/licenses/MIT MIT
 * @link      https://github.com/ezdayo/vpp
 *
 **/

#pragma once

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "customisation/entity.hpp"
#include "customisation/parameter.hpp"
#include "vpp/core/stage.hpp"
#include "vpp/error.hpp"
#include "vpp/scene.hpp"
#include "vpp/util/observability.hpp"

namespace VPP {
namespace Core {

template <typename ...Z> class Pipeline : public Parametrisable {
    public:
        using Stage = VPP::Core::Stage<Z...>;

        Pipeline() noexcept;

        /** Pipelines cannot be copied nor moved */
        Pipeline(const Pipeline& other) = delete;
        Pipeline(Pipeline&& other) = delete;
        Pipeline& operator=(const Pipeline& other) = delete;
        Pipeline& operator=(Pipeline&& other) = delete;
        virtual ~Pipeline() noexcept;

        /* Implementing a default terminate entity implementation aimed at 
         * stopping the pipeline safely before any new initialisation */
        virtual void terminate() noexcept override;

        /* Appending a new stage to the pipeline */
        Pipeline &operator >>(Stage &stage) noexcept;
        
        /* Starting and stopping the pipeline (or keeping it continuing) */
        void start() noexcept;
        void stop() noexcept;
        PARAMETER(Direct, None, Callable, bool) running;

        /* Freezing and unfreezing the pipeline */
        void freeze() noexcept;
        void unfreeze() noexcept;
        PARAMETER(Direct, None, Callable, bool) frozen;

        Util::Notifier<Scene, Z...> broadcast;

        std::function<void (Scene &s, Z&... z) noexcept> finished;
 
    private:
        /* Processing thread for the pipeline */
        inline void launch() noexcept;
        void work(Scene*& s, Z*&... z) noexcept;
        void prepare(Scene*& s, Z*&... z) noexcept;
        Error::Type process(Scene*& s, Z*&... z) noexcept;
 
        Customisation::Error onRunningUpdate(bool yes) noexcept;
        Customisation::Error onFrozenUpdate(bool yes) noexcept;

        /* Storage for internal stages */
        std::vector<std::reference_wrapper<Stage>> stages;

        /* Thread management */
        bool                     run;
        bool                     retry;
        bool                     halt;
        bool                     zombie;
        std::condition_variable  resume;
        std::mutex               suspend;
        std::thread              thread;
};

}  // namespace Core
}  // namespace VPP
