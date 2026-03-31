#pragma once

#include <cstdint>
#include <cstdlib>

// Shared value types used by platform interfaces. These are NOT interfaces
// themselves — they are data carriers that cross the platform boundary.

struct ImageFileBuffer {
    enum EImageType { e_typePNG, e_typeJPG };
    EImageType m_type;
    void* m_pBuffer = nullptr;
    int m_bufferSize = 0;

    [[nodiscard]] int GetType() const { return m_type; }
    [[nodiscard]] void* GetBufferPointer() const { return m_pBuffer; }
    [[nodiscard]] int GetBufferSize() const { return m_bufferSize; }
    [[nodiscard]] bool Allocated() const { return m_pBuffer != nullptr; }
    void Release() {
        std::free(m_pBuffer);
        m_pBuffer = nullptr;
    }
};

struct D3DXIMAGE_INFO {
    int Width;
    int Height;
};

struct XSOCIAL_PREVIEWIMAGE {
    std::uint8_t* pBytes;
    std::uint32_t Pitch;
    std::uint32_t Width;
    std::uint32_t Height;
};
using PXSOCIAL_PREVIEWIMAGE = XSOCIAL_PREVIEWIMAGE*;

struct STRING_VERIFY_RESPONSE {
    std::uint16_t wNumStrings;
    int* pStringResult;
};

enum class EKeyboardResult {
    Pending,
    Cancelled,
    ResultAccept,
    ResultDecline,
};

// Profile-related enums at file scope.
enum class EAwardType {
    Achievement = 0,
    GamerPic,
    Theme,
    AvatarItem,
};

enum class EUpsellType {
    Custom = 0,
    Achievement,
    GamerPic,
    Theme,
    AvatarItem,
};

enum class EUpsellResponse {
    Declined,
    Accepted_NoPurchase,
    Accepted_Purchase,
};

// Backwards-compatible aliases for call sites still using the old
// unscoped names. Remove these as call sites are migrated.
inline constexpr auto EKeyboard_Pending = EKeyboardResult::Pending;
inline constexpr auto EKeyboard_Cancelled = EKeyboardResult::Cancelled;
inline constexpr auto EKeyboard_ResultAccept = EKeyboardResult::ResultAccept;
inline constexpr auto EKeyboard_ResultDecline = EKeyboardResult::ResultDecline;

inline constexpr auto eAwardType_Achievement = EAwardType::Achievement;
inline constexpr auto eAwardType_GamerPic = EAwardType::GamerPic;
inline constexpr auto eAwardType_Theme = EAwardType::Theme;
inline constexpr auto eAwardType_AvatarItem = EAwardType::AvatarItem;

inline constexpr auto eUpsellType_Custom = EUpsellType::Custom;
inline constexpr auto eUpsellType_Achievement = EUpsellType::Achievement;
inline constexpr auto eUpsellType_GamerPic = EUpsellType::GamerPic;
inline constexpr auto eUpsellType_Theme = EUpsellType::Theme;
inline constexpr auto eUpsellType_AvatarItem = EUpsellType::AvatarItem;

inline constexpr auto eUpsellResponse_Declined = EUpsellResponse::Declined;
inline constexpr auto eUpsellResponse_Accepted_NoPurchase = EUpsellResponse::Accepted_NoPurchase;
inline constexpr auto eUpsellResponse_Accepted_Purchase = EUpsellResponse::Accepted_Purchase;
