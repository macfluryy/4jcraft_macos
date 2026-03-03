#ifndef IGGYSTUBS_H
#define IGGYSTUBS_H

#pragma once


#include "../../Windows64/Iggy/include/iggy.h"

#define STUBBED {}

RADEXPFUNC IggyValuePath * RADEXPLINK IggyPlayerRootPath(Iggy *f) {
  STUBBED;
  return nullptr;
}

RADEXPFUNC IggyResult RADEXPLINK IggyPlayerCallMethodRS(Iggy *f, IggyDataValue *result, IggyValuePath *target, IggyName methodname, S32 numargs, IggyDataValue *args) {
  STUBBED;
  return IGGY_RESULT_SUCCESS;
}

RADEXPFUNC void RADEXPLINK IggyPlayerDestroy(Iggy *player) {
  STUBBED;
}
RADEXPFUNC void RADEXPLINK IggyPlayerSetDisplaySize(Iggy *f, S32 w, S32 h) {
  STUBBED;
}

RADEXPFUNC void RADEXPLINK IggyPlayerDrawTilesStart(Iggy *f) {
  STUBBED;
}

RADEXPFUNC void RADEXPLINK IggyPlayerDrawTile(Iggy *f, S32 x0, S32 y0, S32 x1, S32 y1, S32 padding) {
  STUBBED;
}
RADEXPFUNC void RADEXPLINK IggyPlayerDrawTilesEnd(Iggy *f) {
  STUBBED;
}

int thing = 0;

RADEXPFUNC Iggy * RADEXPLINK IggyPlayerCreateFromMemory(
                                void const *           data,
                                U32                    data_size_in_bytes,
                                IggyPlayerConfig      *config) {
  STUBBED;
  return (Iggy*)&thing;
}


RADEXPFUNC void RADEXPLINK IggyPlayerInitializeAndTickRS(Iggy *player) {
  STUBBED;
}

IggyProperties properties;

RADEXPFUNC IggyProperties * RADEXPLINK IggyPlayerProperties(Iggy *player) {
  STUBBED;
  return &properties;
}
RADEXPFUNC void RADEXPLINK IggyPlayerSetUserdata(Iggy *player, void *userdata) {
  STUBBED;
}
RADEXPFUNC IggyName RADEXPLINK IggyPlayerCreateFastName(Iggy *f, IggyUTF16 const *name, S32 len) {
  STUBBED;
  return 0;
}
RADEXPFUNC rrbool RADEXPLINK IggyDebugGetMemoryUseInfo(Iggy *player, IggyLibrary lib, char const *category_string, S32 category_stringlen, S32 iteration, IggyMemoryUseInfo *data) {
  STUBBED;
  return false;
}
RADEXPFUNC rrbool RADEXPLINK IggyPlayerReadyToTick(Iggy *player) {
  STUBBED;
  return false;
}
RADEXPFUNC void RADEXPLINK IggyPlayerTickRS(Iggy *player) {
  STUBBED;
}
RADEXPFUNC void RADEXPLINK IggyPlayerDraw(Iggy *f) {
  STUBBED;
}
RADEXPFUNC void RADEXPLINK IggyMakeEventKey(IggyEvent *event, IggyKeyevent event_type, IggyKeycode keycode, IggyKeyloc keyloc) {
  STUBBED;
}
RADEXPFUNC rrbool RADEXPLINK IggyPlayerDispatchEventRS(Iggy *player, IggyEvent *event, IggyEventResult *result) {
  STUBBED;
  return false;
}
RADEXPFUNC void RADEXPLINK IggyFontRemoveUTF8(const char *fontname, S32 namelen_in_bytes, U32 fontflags) {
  STUBBED;
}
RADEXPFUNC void RADEXPLINK IggyFontInstallBitmapUTF8(const IggyBitmapFontProvider *bmf, const char *fontname, S32 namelen_in_bytes, U32 fontflags) {
  STUBBED;
}
RADEXPFUNC void RADEXPLINK IggyFontSetIndirectUTF8(const char *request_name, S32 request_namelen, U32 request_flags, const char *result_name, S32 result_namelen, U32 result_flags) {
  STUBBED;
}
RADEXPFUNC void RADEXPLINK IggyFontInstallTruetypeUTF8(const void *truetype_storage, S32 ttc_index, const char *fontname, S32 namelen_in_bytes, U32 fontflags) {
  STUBBED;
}
RADEXPFUNC rrbool RADEXPLINK IggyValuePathMakeNameRef(IggyValuePath *result, IggyValuePath *parent, char const *text_utf8) {
  STUBBED;
  return false;
}
RADEXPFUNC IggyResult RADEXPLINK IggyValueGetBooleanRS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, rrbool *result) {
  STUBBED;
  return IGGY_RESULT_SUCCESS;
}
RADEXPFUNC void RADEXPLINK IggyFontInstallTruetypeFallbackCodepointUTF8(const char *fontname, S32 len, U32 fontflags, S32 fallback_codepoint) {
  STUBBED;
}
RADEXPFUNC IggyResult RADEXPLINK IggyValueGetF64RS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, F64 *result) {
  STUBBED;
  return IGGY_RESULT_SUCCESS;
}
RADEXPFUNC rrbool RADEXPLINK IggyValueSetBooleanRS(IggyValuePath *var, IggyName sub_name, char const *sub_name_utf8, rrbool value) {
  STUBBED;
  return false;
}
RADEXPFUNC void RADEXPLINK IggyInit(IggyAllocator *allocator) {
  STUBBED;
}
RADEXPFUNC void RADEXPLINK IggySetWarningCallback(Iggy_WarningFunction *error, void *user_callback_data) {
  STUBBED;
}
RADEXPFUNC void RADEXPLINK IggySetTraceCallbackUTF8(Iggy_TraceFunctionUTF8 *trace_utf8, void *user_callback_data) {
  STUBBED;
}
RADEXPFUNC void RADEXPLINK IggySetFontCachingCalculationBuffer(
    S32 max_chars,
    void *optional_temp_buffer,
    S32 optional_temp_buffer_size_in_bytes) {
  STUBBED;
}
RADEXPFUNC void RADEXPLINK IggySetCustomDrawCallback(Iggy_CustomDrawCallback *custom_draw, void *user_callback_data) {
  STUBBED;
}
RADEXPFUNC void RADEXPLINK IggySetAS3ExternalFunctionCallbackUTF16(Iggy_AS3ExternalFunctionUTF16 *as3_external_function_utf16, void *user_callback_data) {
  STUBBED;
}
RADEXPFUNC void RADEXPLINK IggyMakeEventMouseMove(IggyEvent *event, S32 x, S32 y) {
  STUBBED;
}
RADEXPFUNC void RADEXPLINK IggySetTextureSubstitutionCallbacks(Iggy_TextureSubstitutionCreateCallback *texture_create, Iggy_TextureSubstitutionDestroyCallback *texture_destroy, void *user_callback_data) {
  STUBBED;
}
RADEXPFUNC void * RADEXPLINK IggyPlayerGetUserdata(Iggy *player) {
  STUBBED;
  return 0;
}
RADEXPFUNC  IggyLibrary  RADEXPLINK IggyLibraryCreateFromMemoryUTF16(
                                IggyUTF16 const *      url_utf16_null_terminated,
                                void const *           data,
                                U32                    data_size_in_bytes,
                                IggyPlayerConfig      *config) {
  STUBBED;
  return 0;
}
RADEXPFUNC void RADEXPLINK IggyLibraryDestroy(IggyLibrary lib) {
  STUBBED;
}

#endif // IGGYSTUBS_H