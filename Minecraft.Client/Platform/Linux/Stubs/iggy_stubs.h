#ifndef IGGYSTUBS_H
#define IGGYSTUBS_H

#pragma once


#include "../../Windows64/Iggy/include/iggy.h"

#define STUBBED {}

RADEXPFUNC inline IggyValuePath * RADEXPLINK IggyPlayerRootPath(Iggy *f) {
  STUBBED;
  return nullptr;
}

RADEXPFUNC inline IggyResult RADEXPLINK IggyPlayerCallMethodRS(Iggy *f, IggyDataValue *result, IggyValuePath *target, IggyName methodname, S32 numargs, IggyDataValue *args) {
  STUBBED;
  return IGGY_RESULT_SUCCESS;
}

RADEXPFUNC inline void RADEXPLINK IggyPlayerDestroy(Iggy *player) {
  STUBBED;
}
RADEXPFUNC inline void RADEXPLINK IggyPlayerSetDisplaySize(Iggy *f, S32 w, S32 h) {
  STUBBED;
}

RADEXPFUNC inline void RADEXPLINK IggyPlayerDrawTilesStart(Iggy *f) {
  STUBBED;
}

RADEXPFUNC inline void RADEXPLINK IggyPlayerDrawTile(Iggy *f, S32 x0, S32 y0, S32 x1, S32 y1, S32 padding) {
  STUBBED;
}
RADEXPFUNC inline void RADEXPLINK IggyPlayerDrawTilesEnd(Iggy *f) {
  STUBBED;
}

static int thing = 0;

RADEXPFUNC inline Iggy * RADEXPLINK IggyPlayerCreateFromMemory(
                                void const *           data,
                                U32                    data_size_in_bytes,
                                IggyPlayerConfig      *config) {
  STUBBED;
  return (Iggy*)&thing;
}


RADEXPFUNC inline void RADEXPLINK IggyPlayerInitializeAndTickRS(Iggy *player) {
  STUBBED;
}

static IggyProperties properties;

RADEXPFUNC inline IggyProperties * RADEXPLINK IggyPlayerProperties(Iggy *player) {
  STUBBED;
  return &properties;
}
RADEXPFUNC inline void RADEXPLINK IggyPlayerSetUserdata(Iggy *player, void *userdata) {
  STUBBED;
}
RADEXPFUNC inline IggyName RADEXPLINK IggyPlayerCreateFastName(Iggy *f, IggyUTF16 const *name, S32 len) {
  STUBBED;
  return 0;
}
RADEXPFUNC inline rrbool RADEXPLINK IggyDebugGetMemoryUseInfo(Iggy *player, IggyLibrary lib, char const *category_string, S32 category_stringlen, S32 iteration, IggyMemoryUseInfo *data) {
  STUBBED;
  return false;
}
RADEXPFUNC inline rrbool RADEXPLINK IggyPlayerReadyToTick(Iggy *player) {
  STUBBED;
  return false;
}
RADEXPFUNC inline void RADEXPLINK IggyPlayerTickRS(Iggy *player) {
  STUBBED;
}
RADEXPFUNC inline void RADEXPLINK IggyPlayerDraw(Iggy *f) {
  STUBBED;
}
RADEXPFUNC inline void RADEXPLINK IggyMakeEventKey(IggyEvent *event, IggyKeyevent event_type, IggyKeycode keycode, IggyKeyloc keyloc) {
  STUBBED;
}
RADEXPFUNC inline rrbool RADEXPLINK IggyPlayerDispatchEventRS(Iggy *player, IggyEvent *event, IggyEventResult *result) {
  STUBBED;
  return false;
}
RADEXPFUNC inline void RADEXPLINK IggyFontRemoveUTF8(const char *fontname, S32 namelen_in_bytes, U32 fontflags) {
  STUBBED;
}
RADEXPFUNC inline void RADEXPLINK IggyFontInstallBitmapUTF8(const IggyBitmapFontProvider *bmf, const char *fontname, S32 namelen_in_bytes, U32 fontflags) {
  STUBBED;
}
RADEXPFUNC inline void RADEXPLINK IggyFontSetIndirectUTF8(const char *request_name, S32 request_namelen, U32 request_flags, const char *result_name, S32 result_namelen, U32 result_flags) {
  STUBBED;
}
RADEXPFUNC inline void RADEXPLINK IggyFontInstallTruetypeUTF8(const void *truetype_storage, S32 ttc_index, const char *fontname, S32 namelen_in_bytes, U32 fontflags) {
  STUBBED;
}
RADEXPFUNC inline rrbool RADEXPLINK IggyValuePathMakeNameRef(IggyValuePath *result, IggyValuePath *parent, char const *text_utf8) {
  STUBBED;
  return false;
}
RADEXPFUNC inline IggyResult RADEXPLINK IggyValueGetBooleanRS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, rrbool *result) {
  STUBBED;
  return IGGY_RESULT_SUCCESS;
}
RADEXPFUNC inline void RADEXPLINK IggyFontInstallTruetypeFallbackCodepointUTF8(const char *fontname, S32 len, U32 fontflags, S32 fallback_codepoint) {
  STUBBED;
}
RADEXPFUNC inline IggyResult RADEXPLINK IggyValueGetF64RS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, F64 *result) {
  STUBBED;
  return IGGY_RESULT_SUCCESS;
}
RADEXPFUNC inline rrbool RADEXPLINK IggyValueSetBooleanRS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, rrbool value) {
  STUBBED;
  return true;
}
RADEXPFUNC inline void RADEXPLINK IggyInit(IggyAllocator *allocator) {
  STUBBED;
}
RADEXPFUNC inline void RADEXPLINK IggySetWarningCallback(Iggy_WarningFunction *error, void *user_callback_data) {
  STUBBED;
}
RADEXPFUNC inline void RADEXPLINK IggySetTraceCallbackUTF8(Iggy_TraceFunctionUTF8 *trace_utf8, void *user_callback_data) {
  STUBBED;
}
RADEXPFUNC inline void RADEXPLINK IggySetFontCachingCalculationBuffer(
    S32 max_chars,
    void *optional_temp_buffer,
    S32 optional_temp_buffer_size_in_bytes) {
  STUBBED;
}
RADEXPFUNC inline void RADEXPLINK IggySetCustomDrawCallback(Iggy_CustomDrawCallback *custom_draw, void *user_callback_data) {
  STUBBED;
}
RADEXPFUNC inline void RADEXPLINK IggySetAS3ExternalFunctionCallbackUTF16(Iggy_AS3ExternalFunctionUTF16 *as3_external_function_utf16, void *user_callback_data) {
  STUBBED;
}
RADEXPFUNC inline void RADEXPLINK IggyMakeEventMouseMove(IggyEvent *event, S32 x, S32 y) {
  STUBBED;
}
RADEXPFUNC inline void RADEXPLINK IggySetTextureSubstitutionCallbacks(Iggy_TextureSubstitutionCreateCallback *texture_create, Iggy_TextureSubstitutionDestroyCallback *texture_destroy, void *user_callback_data) {
  STUBBED;
}
RADEXPFUNC inline void * RADEXPLINK IggyPlayerGetUserdata(Iggy *player) {
  STUBBED;
  return 0;
}
RADEXPFUNC inline  IggyLibrary  RADEXPLINK IggyLibraryCreateFromMemoryUTF16(
                                IggyUTF16 const *      url_utf16_null_terminated,
                                void const *           data,
                                U32                    data_size_in_bytes,
                                IggyPlayerConfig      *config) {
  STUBBED;
  return 0;
}
RADEXPFUNC inline void RADEXPLINK IggyLibraryDestroy(IggyLibrary lib) {
  STUBBED;
}

#endif // IGGYSTUBS_H