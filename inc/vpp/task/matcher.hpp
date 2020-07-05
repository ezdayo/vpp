/**
 *
 * @file      vpp/task/matcher.hpp
 *
 * @brief     This is a set of tasks aimed at running matching score evaluations
 *            for objects of different kinds
 *
 * @details   This is the definition of generic matching pattern tasks. It is an
 *            environment to generate the similarity matrix and to extract the 
 *            most likely src-dst pairs
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

#include <opencv2/core/mat.hpp>
#include "vpp/task.hpp"

namespace VPP {
namespace Task {
namespace Matcher {
using Util::containee_object_t;

namespace Estimator {

template <typename Src, typename Dst>
    using Measure 
        = std::function<float (containee_object_t<Src>&, 
                               containee_object_t<Dst>&) noexcept>;

template <typename Src, typename Dst>
class Single : public VPP::Task::Single<Single<Src, Dst>, Src, Dst, float *> {
    public:
        using Parent = VPP::Task::Single<Single<Src, Dst>, Src, Dst, float *>;
        using typename Parent::Mode;
        using Parent::process;
        using Parent::start;

        inline explicit Single(const int mode = Mode::Sync) noexcept 
            : Parent(mode) {}
        inline ~Single() = default;

        inline Error::Type start(Src &src, Dst &dst, cv::Mat &results, 
                                 Measure<Src, Dst> evaluator) noexcept {
            /* Preallocate the results matrix to allow parallel computation and
             * to ensure that it is fully contiguous in memory */ 
            results = std::move(cv::Mat(src.size(), dst.size(), CV_32F));
            float *scores = results.ptr<float>();
            measure = std::move(evaluator);
            return Parent::start(src, dst, scores);
        }

        inline Error::Type process(Src& s, Dst& d, float *&r) noexcept {
            for (auto &src : s) {
                for (auto &dst : d) {
                    *r ++ = measure(static_cast<containee_object_t<Src>&>(src),
                                    static_cast<containee_object_t<Dst>&>(dst));
                }
            }

            return Error::OK;
        }

    protected:
        Measure<Src, Dst> measure;
};

template <typename Src, typename Dst>
class List : public VPP::Tasks::List<List<Src, Dst>, Src, Dst, float *> {
    public:
        using Parent = VPP::Tasks::List<List<Src, Dst>, Src, Dst, float *>;
        using typename Parent::Mode;
        using Parent::process;
        using Parent::start;

        inline explicit List(const int mode = Mode::Async*8) noexcept 
            : Parent(mode) {}
        inline ~List() = default;

        inline Error::Type start(Src &src, Dst &dst, cv::Mat &results, 
                                 Measure<Src, Dst> evaluator) noexcept {
            /* Preallocate the results matrix to allow parallel computation and
             * to ensure that it is fully contiguous in memory */ 
            results = std::move(cv::Mat(src.size(), dst.size(), CV_32F));
            scores = results.ptr<float>();
            measure = std::move(evaluator);
            return Parent::start(src, dst, scores);
        }

        inline bool next(containee_object_t<Src>* &src, Src &s, 
                         Dst &d, float *&r) noexcept {
            r       = scores;
            scores += d.size();
            return Parent::next(src, s, d, r);
        }

        inline Error::Type process(containee_object_t<Src> &src,
                                   Dst& d, float *&r) noexcept {
            for (auto &dst : d) {
                *r ++ = measure(src,
                                static_cast<containee_object_t<Dst>&>(dst));
            }

            return Error::OK;
        }

    protected:
        Measure<Src, Dst> measure;
        float *           scores;
};

template <typename Src, typename Dst>
class Lists : public VPP::Tasks::Lists<Lists<Src, Dst>, Src, Dst, float *> {
    public:
        using Parent = VPP::Tasks::Lists<Lists<Src, Dst>, Src, Dst, float *>;
        using typename Parent::Mode;
        using Parent::process;
        using Parent::start;

        inline explicit Lists(const int mode = Mode::Async*8) noexcept 
            : Parent(mode) {}
        inline ~Lists() = default;

        inline Error::Type start(Src &src, Dst &dst, cv::Mat &results, 
                                 Measure<Src, Dst> evaluator) noexcept {
            /* Preallocate the results matrix to allow parallel computation and
             * to ensure that it is fully contiguous in memory */ 
            results = std::move(cv::Mat(src.size(), dst.size(), CV_32F));
            scores  = results.ptr<float>();
            measure = std::move(evaluator);
            return Parent::start(src, dst, scores);
        }

        inline bool next(containee_object_t<Src>* &src, Src &s, 
                         containee_object_t<Dst>* &dst, Dst &d, 
                         float *&r) noexcept {
            r = scores++;
            return Parent::next(src, s, dst, d, r);
        }

        inline Error::Type process(containee_object_t<Src> &src,
                                   containee_object_t<Dst> &dst, 
                                   float *&r) noexcept {
                *r = measure(src, dst);

            return Error::OK;
        }

    protected:
        Measure<Src, Dst> measure;
        float *           scores;
};

template <typename Src, typename Dst>
class Any : public Parametrisable {
    public:
        using Mode = typename Util::Task::Core::Mode;
    
        enum class Granularity : int {
            /** Process estimation pair by pair */
            MEASURE = -1,
            /** Process estimation by source line */
            ROW     =  0,
            /** Process estimation globally */
            GLOBAL  =  1
        };

        inline explicit
            Any(const int mode_single = Single<Src, Dst>::Mode::Sync,
                const int mode_list   = List<Src, Dst>::Mode::Async*8,
                const int mode_lists  = Lists<Src, Dst>::Mode::Async*8) 
            noexcept : Customisation::Entity("Tasks"), single(mode_single), 
                       list(mode_list), lists(mode_lists) {
            granularity.denominate("granularity")
                       .describe("The granularity of tasks for the matcher "
                                 "estimator: either all for a single task "
                                 "computation, row for one task per row, or "
                                 "measure for one task per measure")
                        .characterise(Customisation::Trait::CONFIGURABLE);
            granularity.define(
                    { { "all",     static_cast<int>(Granularity::GLOBAL) }, 
                      { "row",     static_cast<int>(Granularity::ROW)},
                      { "measure", static_cast<int>(Granularity::MEASURE) } });
            expose(granularity);
        
            granularity = static_cast<int>(Granularity::ROW);
        }
        inline ~Any() = default;

        /** Any matcher evaluator tasks cannot be copied nor moved */
        Any(const Any& other) = delete;
        Any(Any&& other) = delete;
        Any& operator=(const Any& other) = delete;
        Any& operator=(Any&& other) = delete;

        inline Error::Type start(Src &src, Dst &dst, cv::Mat &results, 
                                 Measure<Src, Dst> evaluator) noexcept {
            switch(granularity) {
                case static_cast<int>(Granularity::MEASURE):
                    return lists.start(src, dst, results, std::move(evaluator));
                    break;
                case static_cast<int>(Granularity::ROW):
                    return list.start(src, dst, results, std::move(evaluator));
                    break;
                case static_cast<int>(Granularity::GLOBAL):
                default:
                    return single.start(src, dst, results, 
                                        std::move(evaluator));
                    break;
            }
        }

        inline Error::Type wait() noexcept {
            switch(granularity) {
                case static_cast<int>(Granularity::MEASURE):
                    return lists.wait();
                    break;
                case static_cast<int>(Granularity::ROW):
                    return list.wait();
                    break;
                case static_cast<int>(Granularity::GLOBAL):
                default:
                    return single.wait();
                    break;
            }
        }

        /* Matching evaluator: the scoring evaluator */
        PARAMETER(Mapped, None, Immediate, int) granularity;

    protected:
        Single<Src, Dst> single;
        List<Src, Dst>   list;
        Lists<Src, Dst>  lists;
};

}  // namespace Estimator

/* Matching structure */
struct Match {
    inline Match(int s, int d, float e) noexcept : src(s), dst(d), score(e) {}

    int   src;
    int   dst;
    float score;
};
using Matches = std::vector<Match>;

class Measures {
    public:
        Measures() = default;
        ~Measures() = default;

        /* 
         * Extracting the list of most likely matches which score is above the
         * provided threshold. If exclusive_dst is set, then a given destination
         * can only match one source in the returned matches. Similarly, if
         * exclusive src is set then a given source can only match once in the
         * returned matches
         */
        Matches extract(float threshold, bool exclusive_dst = true, 
                        bool exclusive_src = true) const noexcept;
        
        /* Scores obtained by source for each destination */
        std::vector<float> scores(Match &m) const noexcept;

        /* Scores obtained by peers for the matching destination */
        std::vector<float> peers(Match &m) const noexcept;

    protected:
        cv::Mat measures;
};

template <typename Src, typename Dst,
          template <typename, typename> class Evaluator>
class Generic : public Parametrisable, public Measures {
    public:

        using Mode = typename Evaluator<Src&, Dst&>::Mode;

        template <typename ...Args> inline explicit Generic(Args... args)
            noexcept : Customisation::Entity("Task"), estimator(args...) {
            measure.denominate("measure")
                     .describe("The scoring function for evaluating the "
                               "likeliness of a source representing the same "
                               "object as the destination")
                     .characterise(Customisation::Trait::CONFIGURABLE);
            expose(measure);
            
            threshold.denominate("threshold")
                     .describe("The minimum score for considering a (source, "
                               "destination) pair to be possibly similar and "
                               "consider a match")
                     .characterise(Customisation::Trait::SETTABLE);
            expose(threshold);

            estimator.denominate("estimator");
            expose(estimator);
        }
        inline ~Generic() = default;

        /** Generic matcher tasks cannot be copied nor moved */
        Generic(const Generic& other) = delete;
        Generic(Generic&& other) = delete;
        Generic& operator=(const Generic& other) = delete;
        Generic& operator=(Generic&& other) = delete;

        inline Error::Type estimate(Src &s, Dst &d, 
                                    Estimator::Measure<Src, Dst> e) noexcept {
            /* Keep track of the requested source and destination objects */
            src = &s;
            dst = &d;
            auto error = estimator.start(s, d, measures, std::move(e));
            if (error == Error::NONE) {
                error = estimator.wait();
            }
            return error;
        }
            
        inline Error::Type estimate(Src &s, Dst &d) noexcept {
            Estimator::Measure<Src, Dst> eval =
                *(static_cast<Estimator::Measure<Src,Dst>*>
                                        (static_cast<void *>(measure)));
            return estimate(s, d, std::move(eval)); 
        }

        inline Matches extract(bool exclusive_dst = true, 
                               bool exclusive_src = true) const noexcept {
            return extract(threshold, exclusive_dst, exclusive_src);
        }

        /* Source object reference */
        inline containee_object_t<Src>& source(Match &m) const noexcept {
            return static_cast<containee_object_t<Src>&>(*src[m.src]);
        }

        /* Destination object reference */
        inline containee_object_t<Dst>& destination(Match &m) 
            const noexcept {
            return static_cast<containee_object_t<Dst>&>(*dst[m.dst]);
        }

        /* Matching evaluator: the scoring evaluator */
        PARAMETER(Mapped, None, Immediate, void*) measure;

        /* Matching threshold: the minimum similarity threshold to consider a
         * match */
        PARAMETER(Direct, None, Immediate, float) threshold;

    private:
        /* Use reference to containers not to duplicate container structures */
        Evaluator<Src&, Dst&>       estimator;
        Src *src;
        Dst *dst;
};

}  // namespace Matcher
}  // namespace Task
}  // namespace VPP
