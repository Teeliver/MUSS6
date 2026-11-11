#include "callback_tokens.hpp"
#include <chrono>
#include <limits>
#include <stdexcept>
#include <utility>

namespace mu {
CallbackTokenRegistry::CallbackTokenRegistry(Clock clock, std::uint64_t ttlMs,
                                             std::size_t capacity, std::uint64_t firstSequence)
    : m_clock(std::move(clock)), m_ttlMs(ttlMs), m_nextSequence(firstSequence), m_capacity(capacity)
{
    if (!m_clock) m_clock = [] { return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()); };
    if (!ttlMs || !capacity || !firstSequence) throw std::invalid_argument("callback registry requires nonzero TTL, capacity and sequence");
}

CallbackObjectRef CallbackTokenRegistry::capture(std::uint32_t index) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto it = m_generations.find(index);
    return {index, it == m_generations.end() ? 0 : it->second};
}

bool CallbackTokenRegistry::currentLocked(CallbackObjectRef ref) const
{
    const auto it = m_generations.find(ref.index);
    return ref.generation == (it == m_generations.end() ? 0 : it->second);
}

bool CallbackTokenRegistry::isCurrent(CallbackObjectRef ref) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return currentLocked(ref);
}

void CallbackTokenRegistry::emitFailures(std::vector<FailureWork>& failures)
{
    for (auto& failure : failures) if (failure.callback) failure.callback(failure.reason);
}

void CallbackTokenRegistry::pruneLocked(std::uint64_t now, std::vector<FailureWork>& failures)
{
    for (auto it = m_entries.begin(); it != m_entries.end();) {
        if (now >= it->second.created && now - it->second.created >= m_ttlMs) {
            failures.push_back({std::move(it->second.failure), CallbackCancelReason::Expired});
            it = m_entries.erase(it);
        } else ++it;
    }
}

CallbackWireToken CallbackTokenRegistry::issue(CallbackObjectRef owner, CallbackObjectRef target,
                                               std::uint32_t argument, std::uint32_t channel,
                                               Callback callback, Failure failure, Ready ready)
{
    std::vector<FailureWork> failures;
    CallbackWireToken token;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        const auto now = m_clock();
        pruneLocked(now, failures);
        if (callback && currentLocked(owner) && currentLocked(target) && m_entries.size() < m_capacity) {
            for (std::size_t attempts = 0; attempts <= m_entries.size() + 1; ++attempts) {
                // Exhaustion fails closed. Do not reuse a 64-bit token+nonce sequence.
                if (m_nextSequence == std::numeric_limits<std::uint64_t>::max()) break;
                const auto sequence = m_nextSequence++;
                const std::uint32_t candidate = static_cast<std::uint32_t>(sequence);
                if (!candidate || m_entries.count(candidate)) continue;
                token = {candidate, static_cast<std::uint32_t>(sequence >> 32), argument};
                m_entries.emplace(candidate, Entry{token, owner, target, channel, now,
                    std::move(callback), std::move(failure), std::move(ready)});
                break;
            }
        }
    }
    emitFailures(failures);
    return token;
}

CallbackDisposition CallbackTokenRegistry::dispatch(CallbackWireToken token, std::uint32_t ownerIndex,
                                                     std::uint32_t channel, std::uint32_t coin1,
                                                     std::uint32_t coin2, std::uint32_t coin3)
{
    std::vector<FailureWork> failures;
    Entry entry{};
    CallbackDisposition disposition = CallbackDisposition::Missing;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        pruneLocked(m_clock(), failures);
        const auto it = m_entries.find(token.token);
        if (it != m_entries.end()) {
            const auto& found = it->second;
            if (found.wire.nonce != token.nonce || found.wire.argument != token.argument ||
                found.owner.index != ownerIndex || found.channel != channel) disposition = CallbackDisposition::Mismatch;
            else {
                entry = std::move(it->second);
                m_entries.erase(it); // Consume before dispatch: duplicate responses cannot repeat a purchase.
                disposition = currentLocked(entry.owner) && currentLocked(entry.target) ? CallbackDisposition::Called : CallbackDisposition::Stale;
            }
        }
    }
    emitFailures(failures);
    if (disposition == CallbackDisposition::Stale) {
        if (entry.failure) entry.failure(CallbackCancelReason::OwnerInvalidated);
        return disposition;
    }
    if (disposition == CallbackDisposition::Called) {
        if (entry.ready && !entry.ready()) {
            if (entry.failure) entry.failure(CallbackCancelReason::TargetInvalidated);
            return CallbackDisposition::NotReady;
        }
        entry.callback(coin1, coin2, coin3);
    }
    return disposition;
}

bool CallbackTokenRegistry::cancel(CallbackWireToken token, CallbackCancelReason reason)
{
    Failure failure;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        const auto it = m_entries.find(token.token);
        if (it == m_entries.end() || it->second.wire.nonce != token.nonce) return false;
        failure = std::move(it->second.failure);
        m_entries.erase(it);
    }
    if (failure) failure(reason);
    return true;
}

void CallbackTokenRegistry::invalidateObject(std::uint32_t index)
{
    std::vector<FailureWork> failures;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto& generation = m_generations[index];
        if (generation == std::numeric_limits<std::uint64_t>::max()) throw std::overflow_error("object callback generation exhausted");
        ++generation;
        for (auto it = m_entries.begin(); it != m_entries.end();) {
            if (it->second.owner.index == index || it->second.target.index == index) {
                failures.push_back({std::move(it->second.failure), it->second.owner.index == index ? CallbackCancelReason::OwnerInvalidated : CallbackCancelReason::TargetInvalidated});
                it = m_entries.erase(it);
            } else ++it;
        }
    }
    emitFailures(failures);
}

void CallbackTokenRegistry::cancelAll(CallbackCancelReason reason)
{
    std::vector<FailureWork> failures;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& item : m_entries) failures.push_back({std::move(item.second.failure), reason});
        m_entries.clear();
    }
    emitFailures(failures);
}

void CallbackTokenRegistry::prune()
{
    std::vector<FailureWork> failures;
    { std::lock_guard<std::mutex> lock(m_mutex); pruneLocked(m_clock(), failures); }
    emitFailures(failures);
}

std::size_t CallbackTokenRegistry::pending() const
{
    std::lock_guard<std::mutex> lock(m_mutex); return m_entries.size();
}
}
