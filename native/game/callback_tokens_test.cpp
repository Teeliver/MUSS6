#include "callback_tokens.hpp"
#include <cassert>
#include <cstdint>
#include <vector>

using namespace mu;

int main()
{
    std::uint64_t now = 1000;
    CallbackTokenRegistry registry([&] { return now; }, 100, 2);
    auto buyer = registry.capture(11900), seller = registry.capture(11901);
    int calls = 0;
    std::vector<CallbackCancelReason> failures;
    auto success = [&](std::uint32_t c1, std::uint32_t c2, std::uint32_t c3) { ++calls; assert(c1 == 10 && c2 == 20 && c3 == 30); };
    auto failure = [&](CallbackCancelReason reason) { failures.push_back(reason); (void)registry.pending(); }; // Callback runs outside the mutex.
    auto token = registry.issue(buyer, seller, 204, 3, success, failure);
    assert(token && registry.pending() == 1);
    assert(registry.dispatch({token.token, token.nonce + 1, token.argument}, 11900, 3, 10, 20, 30) == CallbackDisposition::Mismatch);
    assert(registry.dispatch({token.token, token.nonce, 205}, 11900, 3, 10, 20, 30) == CallbackDisposition::Mismatch);
    assert(registry.dispatch(token, 11901, 3, 10, 20, 30) == CallbackDisposition::Mismatch);
    assert(registry.dispatch(token, 11900, 1, 10, 20, 30) == CallbackDisposition::Mismatch);
    assert(calls == 0 && registry.pending() == 1);
    assert(registry.dispatch(token, 11900, 3, 10, 20, 30) == CallbackDisposition::Called);
    assert(registry.dispatch(token, 11900, 3, 10, 20, 30) == CallbackDisposition::Missing);
    assert(calls == 1 && registry.pending() == 0);

    token = registry.issue(buyer, seller, 204, 3, success, failure);
    registry.invalidateObject(buyer.index); // Disconnect and reuse the SAME account/name/index.
    assert(!registry.isCurrent(buyer));
    assert(registry.dispatch(token, 11900, 3, 10, 20, 30) == CallbackDisposition::Missing);
    assert(calls == 1 && failures.back() == CallbackCancelReason::OwnerInvalidated);
    assert(!registry.issue(buyer, seller, 204, 3, success)); // A stale capture cannot issue a new request either.
    buyer = registry.capture(11900);

    token = registry.issue(buyer, seller, 204, 3, success, failure);
    registry.invalidateObject(seller.index);
    assert(failures.back() == CallbackCancelReason::TargetInvalidated && registry.pending() == 0);
    assert(registry.dispatch(token, 11900, 3, 10, 20, 30) == CallbackDisposition::Missing);
    seller = registry.capture(11901);

    token = registry.issue(buyer, seller, 204, 3, success, failure);
    assert(registry.cancel(token));
    assert(failures.back() == CallbackCancelReason::SendFailure);
    assert(!registry.cancel(token));
    assert(registry.dispatch(token, 11900, 3, 10, 20, 30) == CallbackDisposition::Missing);

    token = registry.issue(buyer, seller, 204, 3, success, failure);
    now += 100;
    registry.prune();
    assert(failures.back() == CallbackCancelReason::Expired && registry.pending() == 0);
    assert(registry.dispatch(token, 11900, 3, 10, 20, 30) == CallbackDisposition::Missing);

    auto first = registry.issue(buyer, seller, 204, 3, success, failure);
    auto second = registry.issue(buyer, seller, 205, 3, success, failure);
    assert(first && second && !registry.issue(buyer, seller, 206, 3, success));
    registry.cancelAll();
    assert(failures.back() == CallbackCancelReason::PeerDisconnected && registry.pending() == 0);
    assert(registry.dispatch(first, 11900, 3, 10, 20, 30) == CallbackDisposition::Missing);

    token = registry.issue(buyer, seller, 204, 3, success, failure, [] { return false; });
    assert(registry.dispatch(token, 11900, 3, 10, 20, 30) == CallbackDisposition::NotReady);
    assert(calls == 1 && failures.back() == CallbackCancelReason::TargetInvalidated);

    // After the low DWORD wraps, an old nonce must not invoke a new callback with the same token.
    CallbackTokenRegistry wrapping([&] { return now; }, 100, 10, 0xFFFFFFFFull);
    auto beforeWrap = wrapping.issue(wrapping.capture(1), wrapping.capture(2), 7, 3, [](auto, auto, auto) {});
    auto afterWrap = wrapping.issue(wrapping.capture(1), wrapping.capture(2), 7, 3, [](auto, auto, auto) {});
    assert(beforeWrap.token == 0xFFFFFFFFu && beforeWrap.nonce == 0);
    assert(afterWrap.token == 1 && afterWrap.nonce == 1);
    assert(wrapping.dispatch({1, 0, 7}, 1, 3, 0, 0, 0) == CallbackDisposition::Mismatch);
    assert(wrapping.dispatch(afterWrap, 1, 3, 0, 0, 0) == CallbackDisposition::Called);
    return 0;
}
