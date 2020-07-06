/**
 *
 * @file      vpp/kernel.hpp
 *
 * @brief     This is the VPP kernel description file
 *
 * @details   This file describes the generic structure of a VPP kernel aimed at
 *            efficiently running zone-grained algorithms
 *
 * @author    Olivier Stoltz-Douchet <ezdayo@gmail.com>
 *
 * @copyright (c) 2019-2020 Olivier Stoltz-Douchet
 * @license   http://opensource.org/licenses/MIT MIT
 * @link      https://github.com/ezdayo/vpp
 *
 **/

#pragma once

#include <functional>
#include <list>
#include <vector>

#include "customisation.hpp"
#include "vpp/log.hpp"
#include "vpp/scene.hpp"
#include "vpp/scene.hpp"

namespace VPP {
namespace Kernel {

    class Context {
        public:
            inline Context(Zone &o, const Zone::Copier &c, unsigned int sz = 0) 
                noexcept : original(&o), copier(c), zones() {
                zones.reserve(sz);
                stack(o);
            }
            inline ~Context() = default;

            inline Context(const Context& other) = default;
            inline Context(Context&& other) = default;
            inline Context& operator=(const Context& other) = delete;
            inline Context& operator=(Context&& other) = delete;

            inline bool updated() const noexcept {
                return zones.size() > 1;
            }

            inline int computed() const noexcept {
                return zones.size() - 1;
            }

            inline Zone &stack(const Zone &zone) noexcept {
                zones.push_back(std::move(zone.copy(copier)));
                return zones.back();
            }
            
            inline void flatten() noexcept {
                if (updated()) {
                    /* Update from top to bottom, keeping all information */
                    Zone latest = std::move(zones.back());
                    zones.pop_back();
                    latest.update(zones.back());
                    zones.back() = std::move(latest);

                    /* Loop again (in case) */
                    flatten();
                }
            }

            inline void merge(Context &newer) noexcept {
                newer.flatten();
                stack(newer.zone());
                if (original == nullptr) {
                    original = newer.original;
                }
                newer.zone().invalidate();
                newer.original = nullptr;
            }

            inline const Zone &zone() const noexcept {
                return zones.front();
            }

            inline Zone &zone() noexcept {
                return zones.front();
            }

            inline unsigned int offset_of(int offset) const noexcept {
                ASSERT(offset >= -static_cast<int>(zones.size()) && 
                       offset < static_cast<int>(zones.size()),
                       "Kernel::Context::zone(): invalid offset %d provided "
                       "for a zones vector of size %d", offset,
                       static_cast<int>(zones.size()));
                return (offset < 0) ? zones.size() + offset : offset;
            }

            inline Zone &zone(int offset) noexcept {
                return zones[offset_of(offset)];
            }

            inline const Zone &zone(int offset) const noexcept {
                return zones[offset_of(offset)];
            }

            Zone *              original;
            const Zone::Copier& copier;

        protected:
            std::vector<Zone>   zones;
    };
    
    using Contexts = std::vector<std::reference_wrapper<Context>>;

    /* Class C must be a child class of Context, and E the actual kernel engine
     * type */
    template <typename E, typename C> class Engine : public Parametrisable {
        public:
            using ContextFilter = std::function<bool (const C&) noexcept>;
            using Contexts = std::vector<std::reference_wrapper<C>>;

            /* Default constructor and destructor */
            inline Engine(const Zone::Copier &c, unsigned int sz = 0) noexcept
                : Customisation::Entity("Kernel"), zone_copier(c),
                  stack_size(sz) {
                recall.denominate("recall")
                      .describe("The factor to apply to all predictions scores "
                                "of all historic contexts")
                      .characterise(Customisation::Trait::SETTABLE);
                recall.range(0.0f, 1.0f);
                Customisation::Entity::expose(recall);

                /* Use the global zone recall by default */
                recall = Zone::recall;
            }
            inline ~Engine() = default;

            /* Kernels cannot be copied nor moved */
            inline Engine(const Engine& other) = delete;
            inline Engine(Engine&& other) = delete;
            inline Engine& operator=(const Engine& other) = delete;
            inline Engine& operator=(Engine&& other) = delete;

            inline const C &context(const VPP::Kernel::Context &m) {
                return static_cast<const C &>(m);
            }

            inline C &context(VPP::Kernel::Context &m) {
                return static_cast<C &>(m);
            }

            static inline bool all_contexts(const C& c) noexcept {
                return true;
            }

            static inline bool valid_contexts(const C& c) noexcept {
                return c.zone().valid();
            }

            static inline bool invalid_contexts(const C& c) noexcept {
                return !c.zone().valid();
            }

            static inline bool original_contexts(const C& c) noexcept {
                return c.original != nullptr;
            }

            static inline bool history_contexts(const C& c) noexcept {
                return c.original == nullptr;
            }

            static inline bool updated_contexts(const C& c) noexcept {
                return c.updated();
            }

            /* Tailored accesses to contexts */
            template <typename Cs>
            inline Cs contexts() noexcept {
                return Cs(storage.begin(), storage.end());
            }

            inline Contexts contexts() noexcept {
                return Contexts(storage.begin(), storage.end());
            }

            template <typename Cs>
            inline Cs contexts(const ContextFilter &f) noexcept {
                Cs ctxs;
                for (auto &c : storage) {
                    if (f(c)) {
                        ctxs.emplace_back(c);
                    }
                }
                return ctxs;
            }
            
            inline Contexts contexts(const ContextFilter &f) noexcept {
                Contexts ctxs;
                for (auto &c : storage) {
                    if (f(c)) {
                        ctxs.emplace_back(c);
                    }
                }
                return ctxs;
            }

            /* Tailored accesses to zones */
            inline Zones zones(const ContextFilter &f, int offset) noexcept {
                Zones zs;
                for (auto &c : storage) {
                    if (f(c)) {
                        zs.emplace_back(c.zone(offset));
                    }
                }
                return zs;
            }

            inline Zones zones() noexcept {
                return zones(all_contexts, 0);
            }

            inline Zones zones(const ContextFilter &f) noexcept {
                return zones(f, -1);
            }

            inline Zones zones(int offset) noexcept {
                return zones(all_contexts, offset);
            }

            /* Preparing kernel contexts for new zones */
            template <typename ...Args> 
                inline void prepare(const Zones &zs, Args&... a) noexcept {
                for (auto const &z : zs) {
                    storage.emplace_back(z, zone_copier, stack_size, a...);
                }
            }

            inline void reset() noexcept {
                storage.clear();
            }

            inline void cleanup(Scene &scene,
                                std::vector<Zone> *added = nullptr, 
                                std::vector<Zone> *removed = nullptr) 
                noexcept {
                for (auto it = storage.begin(); it != storage.end(); ) {
                    if (it->zone().invalid()) {
                        if (it->original != nullptr) {
                            it->original->invalidate();
                        }
                        if (removed != nullptr) {
                            removed->push_back(std::move(it->zone()));
                        }
                        it = storage.erase(it);
                    } else {
                        /* If there is already a zone attached to this one,
                         * then update the original zone! */
                        if (it->original != nullptr) {
                            if (it->updated()) {
                                it->flatten();
                                auto z = it->zone().copy(zone_copier);
                                /* Use the recall factor for the non-original
                                 * predictions */
                                it->original->update(z, recall);
                            }
                            /* Get the whole copy of the original zone here */
                            it->zone() = *it->original;
                        } else {
                            it->flatten();
                            scene.mark(it->zone());
                        }
                        /* The original zone references pointer is useless 
                         * after the first pass */
                        it->original = nullptr;
                        ++it;
                        if (added != nullptr) {
                            added->push_back(static_cast<Zone>(it->zone()));
                        }
                    }
                }
                /* Remove invalid zones of scene (if any) */
                scene.extract(Zone::when_invalid);
            }

            /* Recall factor: the factor to apply to the prediction scores of
             * the historic zones */
            PARAMETER(Direct, Saturating, Immediate, float) recall;

        protected:
            const Zone::Copier &zone_copier;
            const unsigned int  stack_size;
            std::list<C>        storage;
    };

}  // namespace Kernel
}  // namespace VPP
