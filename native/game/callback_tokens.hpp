#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace mu {
// These three words replace the old serialized pointers without changing the wire ABI.
struct CallbackWireToken { std::uint32_t token = 0, nonce = 0, argument = 0; explicit operator bool() const { return token != 0; } };
struct CallbackObjectRef { std::uint32_t index = 0; std::uint64_t generation = 0; };
enum class CallbackCancelReason { OwnerInvalidated, TargetInvalidated, SendFailure, Expired, PeerDisconnected };
enum class CallbackDisposition { Called, Missing, Mismatch, Stale, NotReady };

// Registry operations are synchronized; gameplay callbacks/invalidations must be dispatched
// on the native runtime's single gameplay owner thread. No user code runs under its mutex.
class CallbackTokenRegistry {
public:
    using Clock = std::function<std::uint64_t()>;
    using Callback = std::function<void(std::uint32_t, std::uint32_t, std::uint32_t)>;
    using Failure = std::function<void(CallbackCancelReason)>;
    using Ready = std::function<bool()>;
    explicit CallbackTokenRegistry(Clock clock = {}, std::uint64_t ttlMs = 60000,
                                   std::size_t capacity = 4096, std::uint64_t firstSequence = 1);
    CallbackObjectRef capture(std::uint32_t index) const;
    bool isCurrent(CallbackObjectRef ref) const;
    CallbackWireToken issue(CallbackObjectRef owner, CallbackObjectRef target,
                            std::uint32_t argument, std::uint32_t channel,
                            Callback callback, Failure failure = {}, Ready ready = {});
    CallbackDisposition dispatch(CallbackWireToken token, std::uint32_t ownerIndex,
                                 std::uint32_t channel, std::uint32_t coin1,
                                 std::uint32_t coin2, std::uint32_t coin3);
    bool cancel(CallbackWireToken token, CallbackCancelReason reason = CallbackCancelReason::SendFailure);
    void invalidateObject(std::uint32_t index);
    void cancelAll(CallbackCancelReason reason = CallbackCancelReason::PeerDisconnected);
    void prune();
    std::size_t pending() const;
private:
    struct Entry {
        CallbackWireToken wire;
        CallbackObjectRef owner, target;
        std::uint32_t channel;
        std::uint64_t created;
        Callback callback;
        Failure failure;
        Ready ready;
    };
    struct FailureWork { Failure callback; CallbackCancelReason reason; };
    bool currentLocked(CallbackObjectRef ref) const;
    void pruneLocked(std::uint64_t now, std::vector<FailureWork>& failures);
    static void emitFailures(std::vector<FailureWork>& failures);
    Clock m_clock;
    std::uint64_t m_ttlMs, m_nextSequence;
    std::size_t m_capacity;
    mutable std::mutex m_mutex;
    std::unordered_map<std::uint32_t, std::uint64_t> m_generations;
    std::unordered_map<std::uint32_t, Entry> m_entries;
};
}
