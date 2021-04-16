#ifndef CONSTEXPR_MAP_HPP_
#define CONSTEXPR_MAP_HPP_

#include <algorithm>
#include <array>
#include <stdexcept>
#include <tuple>

/** @author Kyle Krol
 *
 *  @brief Simple map intended for use in constexpr contexts.
 *
 *  See this video for details: https://www.youtube.com/watch?v=INn3xa4pMfg.
 */
template <typename K, typename V, std::size_t N>
class ConstexprMap {
  public:
    std::array<std::pair<K, V>, N> const data;

    constexpr ConstexprMap() = delete;
    constexpr ConstexprMap(ConstexprMap const &) = delete;
    constexpr ConstexprMap(ConstexprMap &&) = delete;
    constexpr ConstexprMap &operator=(ConstexprMap const &) = delete;
    constexpr ConstexprMap &operator=(ConstexprMap &&) = delete;

    constexpr auto operator[](K const &k) const {
      auto const it = std::find_if(data.cbegin(), data.cend(), [&](auto const &kv) {
          return (k == kv.first);
        });

      if (it != data.cend()) return it->second;
      else throw std::range_error("Key not found");
    }
};

#endif
