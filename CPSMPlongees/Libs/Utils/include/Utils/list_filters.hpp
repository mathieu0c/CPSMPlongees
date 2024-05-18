#pragma once

#include <QList>
#include <functional>
#include <map>

namespace utils {

template <typename T>
concept ElementIterable = requires(std::ranges::range_value_t<T> x) {
  x.begin();  // must have `x.begin()`
  x.end();    // and `x.end()`
};

template <typename T>
struct Filter {
  std::function<bool(const T &val)> filter;
  bool active{true};
  bool negate{false};
};

template <typename IterType>
void FilterList(IterType begin, IterType end, const std::vector<Filter<typename IterType::value_type>> &filters) {
  std::erase(std::remove_if(begin,  // NOLINT
                            end,
                            [&](const auto &val) {
                              return std::any_of(filters.cbegin(), filters.cend(), [&](const auto &filter) {
                                return filter.active && (filter.negate ? filter.filter(val) : !filter.filter(val));
                              });
                            }),
             end);  // NOLINT
}

template <typename T, typename FilterKeyType>
void FilterList(QList<T> *target_list, const std::map<FilterKeyType, Filter<T>> &filters) {
  target_list->erase(std::remove_if(target_list->begin(),  // NOLINT
                                    target_list->end(),
                                    [&](const auto &val) {
                                      return std::any_of(filters.cbegin(), filters.cend(), [&](const auto &pair) {
                                        const auto &filter = pair.second;
                                        return filter.active &&
                                               (filter.negate ? filter.filter(val) : !filter.filter(val));
                                      });
                                    }),
                     target_list->end());  // NOLINT
}

}  // namespace utils
