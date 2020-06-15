/**
 *
 * @file      vpp/util/observability.hpp
 *
 * @brief     This is the definition for the observability framework
 *
 * @details   Observability is a matter of Notifier class for the observed and
 *            std::functions (to be used with lambda functions) for the
 *            observer, so that the notification can be run in the notified
 *            (observer) context.
 *            Observability is an extension to the observer design pattern, yet
 *            with templated notification objects.
 *            The observer callback function shall not block nor shall it keep
 *            a reference of the notification object. It shall instead extract
 *            the required information and exit ASAP.
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

#include <functional>
#include <list>

namespace Util {
 
template <typename ...O> class Notifier {
    public:
        using Callback = std::function<void(const O&... o, int error) noexcept>;
        using Handle = typename std::list<Callback>::iterator;

        inline Handle connect(Callback c) {
            notifications.emplace_back(std::move(c));
            auto it = notifications.end();
            return (--it);
        };

        inline void disconnect(Handle c) {
            for (auto it = notifications.begin(); 
                      it != notifications.end(); ++it) {
                if (it == c) {
                    notifications.erase(it);
                    return;
                }
            }
        }

        inline void signal(const O&... o, int error) {
            for (auto &c : notifications) {
                c(o..., error);
            }
        }

    private:
        std::list<Callback> notifications;
};

}  // namespace Util
