#pragma once
#include "_main.hxx"
#include <utility>
#include <vector>
#include <iterator>
#include <algorithm>

using std::pair;
using std::vector;
using std::back_inserter;
using std::lower_bound;
using std::find_if;
using std::merge;
using std::inplace_merge;
using std::copy;
using std::min;




template <class T=NONE>
class BitsetPartial {
  int sorted = 0;
  vector<pair<int, T>>  data;
  const int LIMIT = 128;

  // Cute helpers
  private:
  inline int  unsorted() const { return data.size() - sorted; }

  inline auto cbegin()   const { return data.begin(); }
  inline auto cmiddle()  const { return data.begin() + sorted; }
  inline auto cend()     const { return data.end(); }
  inline auto begin()  { return data.begin(); }
  inline auto middle() { return data.begin() + sorted; }
  inline auto end()    { return data.end(); }

  inline int lookupSorted(int id) const {
    auto fl = [](const auto& e, int id) { return e.first <  id; };
    auto fe = [](const auto& e, int id) { return e.first == id; };
    auto it = lower_bound(cbegin(), cmiddle(), id, fl);
    return it==cend() || (*it).first!=id? -1 : it-cbegin();
  }

  inline int lookupUnsorted(int id) const {
    auto fe = [&](const auto& e) { return e.first == id; };
    auto it = find_if(cmiddle(), cend(), fe);
    return it==cend()? -1 : it-cbegin();
  }

  inline int lookup(int id) const {
    int i = lookupSorted(id);
    return i>=0? i : lookupUnsorted(id);
  }

  inline void mergeWithInplace() {
    auto fl = [](const auto& e, const auto& f) { return e.first < f.first; };
    sort(middle(), end(), fl);
    inplace_merge(begin(), middle(), end(), fl);
  }

  inline void mergeAuto() {
    if (unsorted() <= LIMIT) return;
    mergeWithInplace();
    sorted = data.size();
  }

  inline void removeAtSorted(int i) {
    eraseIndex(data, i);
  }

  inline void removeAtUnsorted(int i) {
    int l = data.size()-1;
    sorted = min(sorted, i);
    swap(data[i], data[l]);
    data.pop_back();
  }

  inline void removeAt(int i) {
    int UB = data.size() - LIMIT;
    if (i<UB) removeAtSorted(i);
    else removeAtUnsorted(i);
  }

  // Read as iterable.
  public:
  auto entries() const { return transformIter(data, [](const auto& e) { return e; }); }
  auto keys()    const { return transformIter(data, [](const auto& e) { return e.first; }); }
  auto values()  const { return transformIter(data, [](const auto& e) { return e.second; }); }

  // Read operations.
  public:
  size_t size()      const { return data.size(); }
  bool   has(int id) const { return lookup(id) != -1; }
  T      get(int id) const { int i = lookup(id); return i>=0? data[i].second : T(); }

  // Write operations
  public:
  void clear() {
    data.clear();
  }

  void set(int id, T v) {
    int i = lookup(id);
    if (i<0) return;
    data[i].second = v;
  }

  void add(int id, T v=T()) {
    auto i = lookup(id);
    if (i>=0) return;
    data.push_back({id, v});
    mergeAuto();
  }

  void remove(int id) {
    int i = lookup(id);
    if (i<0) return;
    removeAt(i);
  }
};
