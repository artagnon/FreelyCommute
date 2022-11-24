#pragma once

#include <iostream>

namespace fc::rM::util {
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr void assert_eq(T Expected, T Actual) {
  if (Actual != Expected) {
    std::cout << "Expected: " << std::hex << std::showbase << +Expected
              << std::endl;
    std::cout << "Actual: " << std::hex << std::showbase << +Actual
              << std::endl;
    std::abort();
  }
}

constexpr void assert_msg(bool Condition, const char Msg[]) {
  if (!Condition) {
    std::cout << Msg << std::endl;
    std::abort();
  }
}

void check_rM_hdr(std::ifstream &Stream) {
  constexpr char ExpectedHdr[] = "reMarkable .lines file, version=5          ";
  constexpr size_t Sz = sizeof(ExpectedHdr);
  char ActualHdr[Sz - 1]; // excluding nul terminator
  Stream.read(static_cast<char *>(ActualHdr), Sz - 1);
  util::assert_msg(!strncmp(ActualHdr, ExpectedHdr, Sz - 1),
                   "Header mismatched; please use a valid .rM v5 file");
}

constexpr void check_eof(std::ifstream &Stream) {
  util::assert_msg(Stream.get() == -1, "End of file marker was not found");
  util::assert_msg(Stream.eof(),
                   "Some bytes were found at the end of the stream");
}

constexpr inline uint32_t to_le(char *b) {
  return (b[0] & 0xFF) | ((b[1] << 8) & 0xFF) | ((b[2] << 16) & 0xFF) |
         ((b[3] << 24) & 0xFF);
}

template <typename T, typename U>
constexpr bool decay_equiv_v =
    static_cast<bool>(std::is_same_v<std::decay_t<T>, U>);
} // namespace fc::rM::util
