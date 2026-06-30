#include <cstdio>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include "app/common/src/JavaEdition/JavaServerEntry.h"
#include "app/common/src/JavaEdition/JavaServerList.h"

namespace {

constexpr unsigned int kSeed = 0x5E2531u;
constexpr int kIterations = 1000;

int g_failures = 0;
int g_checks = 0;

void check(bool condition, const char* property, const std::string& detail) {
    ++g_checks;
    if (!condition) {
        ++g_failures;
        std::printf("  FAIL [%s] %s\n", property, detail.c_str());
    }
}

bool entriesEqual(const JavaServerEntry& a, const JavaServerEntry& b) {
    return a.name == b.name && a.host == b.host && a.port == b.port;
}

std::string narrow(const std::wstring& w) {
    std::string s;
    s.reserve(w.size());
    for (wchar_t c : w) s.push_back(c < 128 ? static_cast<char>(c) : '?');
    return s;
}

std::wstring randomName(std::mt19937& rng) {
    static const wchar_t alphabet[] = L"ABCDEFGHIJ klmnop0123456789";
    std::uniform_int_distribution<int> lenDist(0, 16);
    std::uniform_int_distribution<int> chDist(0, static_cast<int>((sizeof(alphabet) / sizeof(wchar_t)) - 2));
    const int len = lenDist(rng);
    std::wstring s;
    for (int i = 0; i < len; ++i) s.push_back(alphabet[chDist(rng)]);
    return s;
}

std::string randomHost(std::mt19937& rng) {
    static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz0123456789.-";
    std::uniform_int_distribution<int> lenDist(1, 20);
    std::uniform_int_distribution<int> chDist(0, static_cast<int>(sizeof(alphabet) - 2));
    const int len = lenDist(rng);
    std::string host;
    for (int i = 0; i < len; ++i) host.push_back(alphabet[chDist(rng)]);
    return host;
}

JavaServerEntry randomEntry(std::mt19937& rng) {
    JavaServerEntry e;
    e.name = randomName(rng);
    e.host = randomHost(rng);
    e.port = static_cast<uint16_t>(std::uniform_int_distribution<int>(1, 65535)(rng));
    return e;
}

JavaServerList buildList(std::mt19937& rng, int n) {
    JavaServerList list;
    for (int i = 0; i < n; ++i) list.add(randomEntry(rng));
    return list;
}

std::string entryDesc(const JavaServerEntry& e) {
    return "{name='" + narrow(e.name) + "',host='" + e.host + "',port=" +
           std::to_string(e.port) + "}";
}

void checkAdd(JavaServerList list, const JavaServerEntry& e) {
    const size_t before = list.size();
    const bool ok = list.add(e);
    check(ok, "add-success", "add of non-empty host returned false: " + entryDesc(e));
    check(list.size() == before + 1, "add-size-plus-one",
          "size " + std::to_string(before) + " -> " + std::to_string(list.size()));
    if (list.size() == before + 1) {
        check(entriesEqual(list.at(list.size() - 1), e), "add-fields-preserved",
              "appended entry mismatch: got " + entryDesc(list.at(list.size() - 1)) +
                  " expected " + entryDesc(e));
    }
}

void checkEdit(JavaServerList list, size_t index, const JavaServerEntry& e) {
    const size_t before = list.size();
    const bool ok = list.edit(index, e);
    check(ok, "edit-success",
          "edit at index " + std::to_string(index) + " (size " + std::to_string(before) +
              ") returned false");
    check(list.size() == before, "edit-size-unchanged",
          "size changed from " + std::to_string(before) + " to " + std::to_string(list.size()));
    if (ok && index < list.size()) {
        check(entriesEqual(list.at(index), e), "edit-fields-updated",
              "edited entry mismatch: got " + entryDesc(list.at(index)) + " expected " +
                  entryDesc(e));
    }
}

void checkRemove(JavaServerList list, size_t index) {
    const size_t before = list.size();
    std::vector<JavaServerEntry> expected;
    for (size_t i = 0; i < before; ++i) {
        if (i != index) expected.push_back(list.at(i));
    }

    const bool ok = list.remove(index);
    check(ok, "remove-success", "remove at index " + std::to_string(index) + " returned false");
    check(list.size() == before - 1, "remove-size-minus-one",
          "size " + std::to_string(before) + " -> " + std::to_string(list.size()));

    if (ok && list.size() == expected.size()) {
        bool orderOk = true;
        for (size_t i = 0; i < expected.size(); ++i) {
            if (!entriesEqual(list.at(i), expected[i])) {
                orderOk = false;
                break;
            }
        }
        check(orderOk, "remove-order-preserved",
              "relative order of survivors changed after removing index " +
                  std::to_string(index));
    }
}

void checkEmptyHostRejected(JavaServerList list, size_t editIndex) {
    JavaServerEntry empty;
    empty.name = L"NoHost";
    empty.host = "";
    empty.port = 25565;

    const std::vector<JavaServerEntry> snapshot = list.entries();
    const bool addOk = list.add(empty);
    check(!addOk, "empty-add-rejected", "add of empty host unexpectedly succeeded");
    const bool editOk = list.edit(editIndex, empty);
    check(!editOk, "empty-edit-rejected",
          "edit with empty host at index " + std::to_string(editIndex) + " unexpectedly succeeded");

    bool unchanged = list.size() == snapshot.size();
    for (size_t i = 0; unchanged && i < snapshot.size(); ++i) {
        if (!entriesEqual(list.at(i), snapshot[i])) unchanged = false;
    }
    check(unchanged, "empty-host-list-unchanged",
          "list mutated after rejected empty-host add/edit");
}

}

int main() {
    std::printf("JavaServerList property tests (seed=0x%X, iters=%d)\n", kSeed, kIterations);

    std::mt19937 rng(kSeed);

    checkAdd(JavaServerList{}, randomEntry(rng));
    checkEmptyHostRejected(JavaServerList{}, 0);

    {
        JavaServerList one = buildList(rng, 1);
        checkEdit(one, 0, randomEntry(rng));
        checkRemove(one, 0);
    }

    std::uniform_int_distribution<int> sizeDist(1, 12);
    for (int i = 0; i < kIterations; ++i) {
        {
            JavaServerList list = buildList(rng, std::uniform_int_distribution<int>(0, 12)(rng));
            checkAdd(list, randomEntry(rng));
        }

        {
            const int n = sizeDist(rng);
            JavaServerList list = buildList(rng, n);
            const size_t idx = static_cast<size_t>(std::uniform_int_distribution<int>(0, n - 1)(rng));
            checkEdit(list, idx, randomEntry(rng));
            checkRemove(list, idx);
        }

        {
            const int n = sizeDist(rng);
            JavaServerList list = buildList(rng, n);
            const size_t idx = static_cast<size_t>(std::uniform_int_distribution<int>(0, n - 1)(rng));
            checkEmptyHostRejected(list, idx);
        }
    }

    std::printf("Ran %d property checks.\n", g_checks);
    if (g_failures == 0) {
        std::printf("ALL PROPERTIES PASSED\n");
        return 0;
    }
    std::printf("%d PROPERTY CHECK(S) FAILED\n", g_failures);
    return 1;
}